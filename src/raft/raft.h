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
#include "state_machine.h"
#include "timer.h"
#include "timer_manager.h"
#include "tracer.h"
#include "vote_manager.h"

namespace vraft {

enum State {
  FOLLOWER = 0,
  CANDIDATE,
  LEADER,
};

char *StateToStr(enum State state);
void Tick(Timer *timer);
void Elect(Timer *timer);
void ElectRpc(Timer *timer);
void HeartBeat(Timer *timer);

class Raft;
using RaftUPtr = std::unique_ptr<Raft>;

class Raft final {
 public:
  Raft(const std::string &path, const RaftConfig &rc);
  ~Raft();
  Raft(const Raft &r) = delete;
  Raft &operator=(const Raft &r) = delete;

  // life cycle
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

  // send message
  int32_t DoPing(uint64_t dest);
  int32_t DoRequestVote(uint64_t dest);
  int32_t DoAppendEntries(uint64_t dest);
  int32_t DoInstallSnapshot(uint64_t dest);

  // utils
  int16_t Id() { return config_mgr_.Current().me.id(); }
  RaftAddr Me() { return config_mgr_.Current().me; }
  std::vector<RaftAddr> Peers() { return config_mgr_.Current().peers; }
  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);

  // set
  void set_send(SendFunc func) { send_ = func; }
  void set_make_timer(MakeTimerFunc func) { make_timer_ = func; }

 private:
  int32_t InitConfig();
  RaftIndex LastIndex();
  RaftTerm LastTerm();
  bool VoteForMyself();
  void StepDown(RaftTerm new_term);
  void BecomeLeader();
  void AppendNoop();

 private:
  // path
  std::string home_path_;
  std::string conf_path_;
  std::string meta_path_;
  std::string log_path_;
  std::string sm_path_;

  // in-memory data
  enum State state_;
  RaftIndex commit_;
  RaftIndex last_apply_;
  RaftAddr leader_;

  // persistent data
  RaftLog log_;
  SolidData meta_;

  // manager
  ConfigManager config_mgr_;
  IndexManager index_mgr_;
  VoteManager vote_mgr_;
  TimerManager timer_mgr_;

  // state machine
  StateMachine sm_;

  // func
  SendFunc send_;
  MakeTimerFunc make_timer_;

  friend void Tick(Timer *timer);
  friend void Elect(Timer *timer);
  friend void ElectRpc(Timer *timer);
  friend void HeartBeat(Timer *timer);
};

inline Raft::~Raft() {}

}  // namespace vraft

#endif
