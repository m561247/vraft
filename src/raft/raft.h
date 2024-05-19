#ifndef VRAFT_RAFT_H_
#define VRAFT_RAFT_H_

#include <functional>
#include <vector>

#include "append_entries.h"
#include "append_entries_reply.h"
#include "checker.h"
#include "config.h"
#include "config_manager.h"
#include "index_manager.h"
#include "install_snapshot.h"
#include "install_snapshot_reply.h"
#include "nlohmann/json.hpp"
#include "ping.h"
#include "ping_reply.h"
#include "raft_addr.h"
#include "raft_log.h"
#include "request_vote.h"
#include "request_vote_reply.h"
#include "simple_random.h"
#include "solid_data.h"
#include "timer.h"
#include "tracer.h"
#include "vote_manager.h"

namespace vraft {

using SendFunc =
    std::function<int32_t(uint64_t dest, const char *buf, unsigned int size)>;
using MakeTimerFunc = std::function<TimerPtr(
    uint64_t to_ms, uint64_t rp_ms, const TimerFunctor &func, void *data)>;

class Raft;
using RaftUPtr = std::unique_ptr<Raft>;

enum State {
  FOLLOWER = 0,
  CANDIDATE,
  LEADER,
};

char *StateToStr(enum State state);
void PingTimerFunc(Timer *timer);
void ElectionTimerFunc(Timer *timer);
void HeartbeatTimerFunc(Timer *timer);

class Raft final {
 public:
  Raft(const std::string &path, const RaftConfig &rc);
  ~Raft();
  Raft(const Raft &r) = delete;
  Raft &operator=(const Raft &r) = delete;

  // start, stop
  int32_t Start();
  int32_t Stop();
  void Init();

  // on message
  int32_t OnPing(struct Ping &msg);
  int32_t OnPingReply(struct PingReply &msg);
  int32_t OnRequestVote(struct RequestVote &msg);
  int32_t OnRequestVoteReply(struct RequestVoteReply &msg);
  int32_t OnAppendEntries(struct AppendEntries &msg);
  int32_t OnAppendEntriesReply(struct AppendEntriesReply &msg);
  int32_t OnInstallSnapshot(struct InstallSnapshot &msg);
  int32_t OnInstallSnapshotReply(struct InstallSnapshotReply &msg);

  // utils
  int16_t Id() { return config_mgr_.Current().me.id(); }
  RaftAddr Me() { return config_mgr_.Current().me; }
  std::vector<RaftAddr> Peers() { return config_mgr_.Current().peers; }

  void set_send(SendFunc func) { send_ = func; }
  void set_make_timer(MakeTimerFunc func) { make_timer_ = func; }

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);

 private:
  int32_t InitConfig();
  RaftIndex LastIndex();
  RaftTerm LastTerm();

 private:
  // path
  std::string home_path_;
  std::string conf_path_;
  std::string meta_path_;
  std::string log_path_;

  // memory data
  enum State state_;
  RaftIndex commit_;
  RaftAddr leader_;

  // persistent data
  RaftLog log_;
  SolidData meta_;

  // manager
  ConfigManager config_mgr_;
  IndexManager index_mgr_;
  VoteManager vote_mgr_;

  // timer
  uint32_t ping_timer_ms_;
  uint32_t election_timer_ms_;
  uint32_t heartbeat_timer_ms_;
  SimpleRandom random_election_ms_;
  TimerPtr ping_timer_;
  TimerPtr election_timer_;
  TimerPtr heartbeat_timer_;

  // func
  SendFunc send_;
  MakeTimerFunc make_timer_;

  friend void PingTimerFunc(Timer *timer);
  friend void ElectionTimerFunc(Timer *timer);
  friend void HeartbeatTimerFunc(Timer *timer);
};

inline Raft::~Raft() {}

}  // namespace vraft

#endif
