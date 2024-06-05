#include <algorithm>
#include <cstdlib>
#include <fstream>

#include "clock.h"
#include "raft.h"
#include "raft_server.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

int32_t Raft::OnRequestVote(struct RequestVote &msg) {
  if (started_) {
    Tracer tracer(this, true);
    tracer.PrepareState0();
    tracer.PrepareEvent(kRecv, msg.ToJsonString(false, true));

    RaftIndex last_index = LastIndex();
    RaftTerm last_term = LastTerm();
    bool log_ok =
        ((msg.last_log_term > last_term) ||
         (msg.last_log_term == last_term && msg.last_log_index >= last_index));

    if (msg.term > meta_.term()) {
      StepDown(msg.term);
    }

    if (msg.term == meta_.term()) {
      if (log_ok && meta_.vote() == 0) {
        StepDown(meta_.term());

        // reset election
        timer_mgr_.StopRequestVote();
        timer_mgr_.AgainElection();

        // vote
        meta_.SetVote(msg.src.ToU64());
      }
    }

    RequestVoteReply reply;
    reply.src = msg.dest;
    reply.dest = msg.src;
    reply.term = meta_.term();
    reply.granted =
        (msg.term == meta_.term() && meta_.vote() == msg.src.ToU64());
    SendRequestVoteReply(reply);
    tracer.PrepareEvent(kSend, reply.ToJsonString(false, true));

    tracer.PrepareState1();
    tracer.Finish();
  }
  return 0;
}

int32_t Raft::OnRequestVoteReply(struct RequestVoteReply &msg) {
  if (started_) {
    Tracer tracer(this, true);
    tracer.PrepareState0();
    tracer.PrepareEvent(kRecv, msg.ToJsonString(false, true));

    if (msg.term > meta_.term()) {
      StepDown(msg.term);

    } else {
      // close rpc timer
      timer_mgr_.StopRequestVote(msg.src.ToU64());

      if (msg.granted) {
        // get vote
        vote_mgr_.GetVote(msg.src.ToU64());

        if (vote_mgr_.Majority(IfSelfVote()) && state_ != LEADER) {
          BecomeLeader();
        }
      }
    }

    tracer.PrepareState1();
    tracer.Finish();
  }
  return 0;
}

}  // namespace vraft
