#include <algorithm>
#include <cstdlib>
#include <fstream>

#include "clock.h"
#include "raft.h"
#include "raft_server.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

int32_t Raft::OnAppendEntries(struct AppendEntries &msg) {
  if (started_) {
    AppendEntriesReply reply;
    reply.src = msg.dest;
    reply.dest = msg.src;
    reply.term = meta_.term();
    reply.success = false;
    reply.last_log_index = LastIndex();
    reply.num_entries = msg.entries.size();

    if (msg.term < meta_.term()) {
      SendAppendEntriesReply(reply);
      return 0;
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
      return 0;
    }

    assert(msg.pre_log_index <= LastIndex());
    if (GetTerm(msg.pre_log_index) != msg.pre_log_term) {  // reject
      SendAppendEntriesReply(reply);
      return 0;
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
  }
  return 0;
}

int32_t Raft::OnAppendEntriesReply(struct AppendEntriesReply &msg) {
  if (started_) {
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
  }
  return 0;
}

}  // namespace vraft
