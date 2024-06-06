#include <algorithm>
#include <cstdlib>
#include <fstream>

#include "clock.h"
#include "raft.h"
#include "raft_server.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

/********************************************************************************************
\* Server i times out and starts a new election.
Timeout(i) == /\ state[i] \in {Follower, Candidate}
              /\ state' = [state EXCEPT ![i] = Candidate]
              /\ currentTerm' = [currentTerm EXCEPT ![i] = currentTerm[i] + 1]
              \* Most implementations would probably just set the local vote
              \* atomically, but messaging localhost for it is weaker.
              /\ votedFor' = [votedFor EXCEPT ![i] = Nil]
              /\ votesResponded' = [votesResponded EXCEPT ![i] = {}]
              /\ votesGranted'   = [votesGranted EXCEPT ![i] = {}]
              /\ voterLog'       = [voterLog EXCEPT ![i] = [j \in {} |-> <<>>]]
              /\ UNCHANGED <<messages, leaderVars, logVars>>
********************************************************************************************/
void Elect(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data());
  assert(r->state_ == FOLLOWER || r->state_ == CANDIDATE);

  Tracer tracer(r, true);
  tracer.PrepareState0();

  r->meta_.IncrTerm();
  r->state_ = CANDIDATE;
  r->leader_ = RaftAddr(0);
  r->vote_mgr_.Clear();

  // vote for myself
  r->meta_.SetVote(r->Me().ToU64());

  // only myself, become leader
  if (r->vote_mgr_.QuorumAll(r->IfSelfVote())) {
    r->BecomeLeader();
    return;
  }

  tracer.PrepareEvent(kTimer, "election-timer timeout");
  tracer.PrepareState1();
  tracer.Finish();

  // start request-vote
  r->timer_mgr_.StartRequestVote();

  // reset election timer
  r->timer_mgr_.AgainElection();
}

/********************************************************************************************
\* Candidate i sends j a RequestVote request.
RequestVote(i, j) ==
    /\ state[i] = Candidate
    /\ j \notin votesResponded[i]
    /\ Send([mtype         |-> RequestVoteRequest,
             mterm         |-> currentTerm[i],
             mlastLogTerm  |-> LastTerm(log[i]),
             mlastLogIndex |-> Len(log[i]),
             msource       |-> i,
             mdest         |-> j])
    /\ UNCHANGED <<serverVars, candidateVars, leaderVars, logVars>>
********************************************************************************************/
void RequestVoteRpc(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data());
  assert(r->state_ == CANDIDATE);

  Tracer tracer(r, true);
  tracer.PrepareState0();

  int32_t rv = r->SendRequestVote(timer->dest_addr(), &tracer);
  assert(rv == 0);

  tracer.PrepareState1();
  tracer.Finish();
}

int32_t Raft::SendRequestVote(uint64_t dest, Tracer *tracer) {
  RequestVote msg;
  msg.src = Me();
  msg.dest = RaftAddr(dest);
  msg.term = meta_.term();

  msg.last_log_index = LastIndex();
  msg.last_log_term = LastTerm();

  std::string body_str;
  int32_t bytes = msg.ToString(body_str);

  MsgHeader header;
  header.body_bytes = bytes;
  header.type = kRequestVote;
  std::string header_str;
  header.ToString(header_str);

  if (send_) {
    header_str.append(std::move(body_str));
    send_(dest, header_str.data(), header_str.size());

    if (tracer != nullptr) {
      tracer->PrepareEvent(kSend, msg.ToJsonString(false, true));
    }
  }
  return 0;
}

/********************************************************************************************
\* Leader i sends j an AppendEntries request containing up to 1 entry.
\* While implementations may want to send more than 1 at a time, this spec uses
\* just 1 because it minimizes atomic regions without loss of generality.
AppendEntries(i, j) ==
    /\ i /= j
    /\ state[i] = Leader
    /\ LET prevLogIndex == nextIndex[i][j] - 1
           prevLogTerm == IF prevLogIndex > 0 THEN
                              log[i][prevLogIndex].term
                          ELSE
                              0
           \* Send up to 1 entry, constrained by the end of the log.
           lastEntry == Min({Len(log[i]), nextIndex[i][j]})
           entries == SubSeq(log[i], nextIndex[i][j], lastEntry)
       IN Send([mtype          |-> AppendEntriesRequest,
                mterm          |-> currentTerm[i],
                mprevLogIndex  |-> prevLogIndex,
                mprevLogTerm   |-> prevLogTerm,
                mentries       |-> entries,
                \* mlog is used as a history variable for the proof.
                \* It would not exist in a real implementation.
                mlog           |-> log[i],
                mcommitIndex   |-> Min({commitIndex[i], lastEntry}),
                msource        |-> i,
                mdest          |-> j])
    /\ UNCHANGED <<serverVars, candidateVars, leaderVars, logVars>>
********************************************************************************************/
void HeartBeat(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data());
  assert(r->state_ == LEADER);

  Tracer tracer(r, true);
  tracer.PrepareState0();

  tracer.PrepareEvent(kTimer, "heartbeat-timer timeout");
  int32_t rv = r->SendAppendEntries(timer->dest_addr(), &tracer);
  assert(rv == 0);

  tracer.PrepareState1();
  tracer.Finish();
}

int32_t Raft::SendAppendEntries(uint64_t dest, Tracer *tracer) {
  AppendEntries msg;
  msg.src = Me();
  msg.dest = RaftAddr(dest);
  msg.term = meta_.term();

  msg.pre_log_index = LastIndex() - 1;
  msg.pre_log_term = GetTerm(msg.pre_log_index);
  msg.commit_index = commit_;

  std::string body_str;
  int32_t bytes = msg.ToString(body_str);

  MsgHeader header;
  header.body_bytes = bytes;
  header.type = kAppendEntries;
  std::string header_str;
  header.ToString(header_str);

  if (send_) {
    header_str.append(std::move(body_str));
    send_(dest, header_str.data(), header_str.size());

    if (tracer != nullptr) {
      tracer->PrepareEvent(kSend, msg.ToJsonString(false, true));
    }
  }

  return 0;
}

int32_t Raft::SendInstallSnapshot(uint64_t dest, Tracer *tracer) { return 0; }

/********************************************************************************************
\* Leader i receives a client request to add v to the log.
ClientRequest(i, v) ==
    /\ state[i] = Leader
    /\ LET entry == [term  |-> currentTerm[i],
                     value |-> v]
           newLog == Append(log[i], entry)
       IN  log' = [log EXCEPT ![i] = newLog]
    /\ UNCHANGED <<messages, serverVars, candidateVars,
                   leaderVars, commitIndex>>
********************************************************************************************/
int32_t Propose(std::string value) { return 0; }

/********************************************************************************************
\* The term of the last entry in a log, or 0 if the log is empty.
LastTerm(xlog) == IF Len(xlog) = 0 THEN 0 ELSE xlog[Len(xlog)].term
********************************************************************************************/
RaftTerm Raft::LastTerm() {
  RaftIndex snapshot_last = sm_.LastIndex();
  RaftIndex log_last = log_.Last();

  if (log_last >= snapshot_last) {  // log is newer
    if (log_last == 0) {            // no log, no snapshot
      return 0;

    } else {                         // has log
      return log_.LastMeta()->term;  // return last log term
    }

  } else {  // snapshot is newer
    return sm_.LastTerm();
  }
}

/********************************************************************************************
\* Any RPC with a newer term causes the recipient to advance its term first.
UpdateTerm(i, j, m) ==
    /\ m.mterm > currentTerm[i]
    /\ currentTerm'    = [currentTerm EXCEPT ![i] = m.mterm]
    /\ state'          = [state       EXCEPT ![i] = Follower]
    /\ votedFor'       = [votedFor    EXCEPT ![i] = Nil]
       \* messages is unchanged so m can be processed further.
    /\ UNCHANGED <<messages, candidateVars, leaderVars, logVars>>
********************************************************************************************/
void Raft::StepDown(RaftTerm new_term) {
  assert(meta_.term() <= new_term);
  if (meta_.term() < new_term) {  // newer term
    meta_.SetTerm(new_term);
    meta_.SetVote(0);
    leader_ = RaftAddr(0);
    state_ = FOLLOWER;

  } else {  // equal term
    if (state_ != FOLLOWER) {
      state_ = FOLLOWER;
    }
  }

  // close heartbeat timer
  timer_mgr_.StopHeartBeat();

  // start election timer
  timer_mgr_.StopRequestVote();
  timer_mgr_.AgainElection();
}

/********************************************************************************************
\* Candidate i transitions to leader.
BecomeLeader(i) ==
    /\ state[i] = Candidate
    /\ votesGranted[i] \in Quorum
    /\ state'      = [state EXCEPT ![i] = Leader]
    /\ nextIndex'  = [nextIndex EXCEPT ![i] =
                         [j \in Server |-> Len(log[i]) + 1]]
    /\ matchIndex' = [matchIndex EXCEPT ![i] =
                         [j \in Server |-> 0]]
    /\ elections'  = elections \cup
                         {[eterm     |-> currentTerm[i],
                           eleader   |-> i,
                           elog      |-> log[i],
                           evotes    |-> votesGranted[i],
                           evoterLog |-> voterLog[i]]}
    /\ UNCHANGED <<messages, currentTerm, votedFor, candidateVars, logVars>>
********************************************************************************************/
void Raft::BecomeLeader() {
  assert(state_ == CANDIDATE);
  state_ = LEADER;
  leader_ = Me().ToU64();

  // stop election timer
  timer_mgr_.StopElection();
  timer_mgr_.StopRequestVote();

  // reset index manager
  index_mgr_.ResetNext(LastIndex() + 1);
  index_mgr_.ResetMatch(0);

  // append noop
  AppendNoop();

  // start heartbeat timer
  timer_mgr_.StartHeartBeat();
}

/********************************************************************************************
\* Leader i advances its commitIndex.
\* This is done as a separate step from handling AppendEntries responses,
\* in part to minimize atomic regions, and in part so that leaders of
\* single-server clusters are able to mark entries committed.
AdvanceCommitIndex(i) ==
    /\ state[i] = Leader
    /\ LET \* The set of servers that agree up through index.
           Agree(index) == {i} \cup {k \in Server :
                                         matchIndex[i][k] >= index}
           \* The maximum indexes for which a quorum agrees
           agreeIndexes == {index \in 1..Len(log[i]) :
                                Agree(index) \in Quorum}
           \* New value for commitIndex'[i]
           newCommitIndex ==
              IF /\ agreeIndexes /= {}
                 /\ log[i][Max(agreeIndexes)].term = currentTerm[i]
              THEN
                  Max(agreeIndexes)
              ELSE
                  commitIndex[i]
       IN commitIndex' = [commitIndex EXCEPT ![i] = newCommitIndex]
    /\ UNCHANGED <<messages, serverVars, candidateVars, leaderVars, log>>
********************************************************************************************/
void Raft::MaybeCommit() {}

}  // namespace vraft
