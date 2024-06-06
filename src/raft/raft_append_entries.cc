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
\* Server i receives an AppendEntries request from server j with
\* m.mterm <= currentTerm[i]. This just handles m.entries of length 0 or 1, but
\* implementations could safely accept more by treating them the same as
\* multiple independent requests of 1 entry.
HandleAppendEntriesRequest(i, j, m) ==
    LET logOk == \/ m.mprevLogIndex = 0
                 \/ /\ m.mprevLogIndex > 0
                    /\ m.mprevLogIndex <= Len(log[i])
                    /\ m.mprevLogTerm = log[i][m.mprevLogIndex].term
    IN /\ m.mterm <= currentTerm[i]
       /\ \/ /\ \* reject request
                \/ m.mterm < currentTerm[i]
                \/ /\ m.mterm = currentTerm[i]
                   /\ state[i] = Follower
                   /\ \lnot logOk
             /\ Reply([mtype           |-> AppendEntriesResponse,
                       mterm           |-> currentTerm[i],
                       msuccess        |-> FALSE,
                       mmatchIndex     |-> 0,
                       msource         |-> i,
                       mdest           |-> j],
                       m)
             /\ UNCHANGED <<serverVars, logVars>>
          \/ \* return to follower state
             /\ m.mterm = currentTerm[i]
             /\ state[i] = Candidate
             /\ state' = [state EXCEPT ![i] = Follower]
             /\ UNCHANGED <<currentTerm, votedFor, logVars, messages>>
          \/ \* accept request
             /\ m.mterm = currentTerm[i]
             /\ state[i] = Follower
             /\ logOk
             /\ LET index == m.mprevLogIndex + 1
                IN \/ \* already done with request
                       /\ \/ m.mentries = << >>
                          \/ /\ m.mentries /= << >>
                             /\ Len(log[i]) >= index
                             /\ log[i][index].term = m.mentries[1].term
                          \* This could make our commitIndex decrease (for
                          \* example if we process an old, duplicated request),
                          \* but that doesn't really affect anything.
                       /\ commitIndex' = [commitIndex EXCEPT ![i] =
                                              m.mcommitIndex]
                       /\ Reply([mtype           |-> AppendEntriesResponse,
                                 mterm           |-> currentTerm[i],
                                 msuccess        |-> TRUE,
                                 mmatchIndex     |-> m.mprevLogIndex +
                                                     Len(m.mentries),
                                 msource         |-> i,
                                 mdest           |-> j],
                                 m)
                       /\ UNCHANGED <<serverVars, log>>
                   \/ \* conflict: remove 1 entry
                       /\ m.mentries /= << >>
                       /\ Len(log[i]) >= index
                       /\ log[i][index].term /= m.mentries[1].term
                       /\ LET new == [index2 \in 1..(Len(log[i]) - 1) |->
                                          log[i][index2]]
                          IN log' = [log EXCEPT ![i] = new]
                       /\ UNCHANGED <<serverVars, commitIndex, messages>>
                   \/ \* no conflict: append entry
                       /\ m.mentries /= << >>
                       /\ Len(log[i]) = m.mprevLogIndex
                       /\ log' = [log EXCEPT ![i] =
                                      Append(log[i], m.mentries[1])]
                       /\ UNCHANGED <<serverVars, commitIndex, messages>>
       /\ UNCHANGED <<candidateVars, leaderVars>>
********************************************************************************************/
int32_t Raft::OnAppendEntries(struct AppendEntries &msg) {
  if (started_) {
    Tracer tracer(this, true);
    tracer.PrepareState0();
    tracer.PrepareEvent(kRecv, msg.ToJsonString(false, true));

    AppendEntriesReply reply;
    reply.src = msg.dest;
    reply.dest = msg.src;
    reply.term = meta_.term();
    reply.success = false;
    reply.last_log_index = LastIndex();
    reply.num_entries = msg.entries.size();

    if (msg.term < meta_.term()) {
      SendAppendEntriesReply(reply);
      tracer.PrepareEvent(kSend, reply.ToJsonString(false, true));
      goto end;
    }

    if (msg.term > meta_.term()) {
      reply.term = msg.term;
    }

    StepDown(msg.term);
    if (leader_.ToU64() == 0) {
      leader_ = msg.src;
    } else {
      assert(leader_.ToU64() == msg.src.ToU64());
    }

    if (msg.pre_log_index > LastIndex()) {  // reject
      SendAppendEntriesReply(reply);
      tracer.PrepareEvent(kSend, reply.ToJsonString(false, true));
      goto end;
    }

    assert(msg.pre_log_index <= LastIndex());
    if (GetTerm(msg.pre_log_index) != msg.pre_log_term) {  // reject
      SendAppendEntriesReply(reply);
      tracer.PrepareEvent(kSend, reply.ToJsonString(false, true));
      goto end;
    }

    // make log same

    reply.success = true;
    reply.last_log_index = LastIndex();
    if (commit_ < msg.commit_index) {
      commit_ = msg.commit_index;
    }

    // reset election timer
    timer_mgr_.StopRequestVote();
    timer_mgr_.AgainElection();

    SendAppendEntriesReply(reply);
    tracer.PrepareEvent(kSend, reply.ToJsonString(false, true));

  end:
    tracer.PrepareState1();
    tracer.Finish();
  }
  return 0;
}

int32_t Raft::SendAppendEntriesReply(AppendEntriesReply &msg) {
  std::string body_str;
  int32_t bytes = msg.ToString(body_str);

  MsgHeader header;
  header.body_bytes = bytes;
  header.type = kAppendEntriesReply;
  std::string header_str;
  header.ToString(header_str);

  if (send_) {
    header_str.append(std::move(body_str));
    send_(msg.dest.ToU64(), header_str.data(), header_str.size());
  }

  return 0;
}

/********************************************************************************************
\* Server i receives an AppendEntries response from server j with
\* m.mterm = currentTerm[i].
HandleAppendEntriesResponse(i, j, m) ==
    /\ m.mterm = currentTerm[i]
    /\ \/ /\ m.msuccess \* successful
          /\ nextIndex'  = [nextIndex  EXCEPT ![i][j] = m.mmatchIndex + 1]
          /\ matchIndex' = [matchIndex EXCEPT ![i][j] = m.mmatchIndex]
       \/ /\ \lnot m.msuccess \* not successful
          /\ nextIndex' = [nextIndex EXCEPT ![i][j] =
                               Max({nextIndex[i][j] - 1, 1})]
          /\ UNCHANGED <<matchIndex>>
    /\ Discard(m)
    /\ UNCHANGED <<serverVars, candidateVars, logVars, elections>>
********************************************************************************************/
int32_t Raft::OnAppendEntriesReply(struct AppendEntriesReply &msg) {
  if (started_) {
    Tracer tracer(this, true);
    tracer.PrepareState0();
    tracer.PrepareEvent(kRecv, msg.ToJsonString(false, true));

    assert(state_ == LEADER);
    if (msg.term > meta_.term()) {
      StepDown(msg.term);

    } else {
      assert(msg.term == meta_.term());

      // reset hb timer ?

      if (msg.success) {
        RaftIndex pre_index = index_mgr_.GetNext(msg.src) - 1;
        if (index_mgr_.GetMatch(msg.src) > pre_index + msg.num_entries) {
          // maybe pipeline ?

        } else {
          index_mgr_.SetMatch(msg.src, pre_index + msg.num_entries);
          MaybeCommit();
        }

        index_mgr_.SetNext(msg.src, index_mgr_.GetMatch(msg.src) + 1);

      } else {
        if (index_mgr_.GetNext(msg.src) > 1) {
          index_mgr_.DecrNext(msg.src);
        }

        if (index_mgr_.GetNext(msg.src) > msg.last_log_index + 1) {
          index_mgr_.SetNext(msg.src, msg.last_log_index + 1);
        }
      }
    }

    tracer.PrepareState1();
    tracer.Finish();
  }
  return 0;
}

}  // namespace vraft
