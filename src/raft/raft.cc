#include "raft.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>

#include "clock.h"
#include "raft_server.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

char *StateToStr(enum State state) {
  switch (state) {
    case FOLLOWER:
      return "FOLLOWER";
      break;
    case CANDIDATE:
      return "CANDIDATE";
      break;
    case LEADER:
      return "LEADER";
      break;
    default:
      assert(0);
  }
}

void PingPeers(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data);
  for (auto &dest_addr : r->Peers()) {
    r->DoPing(dest_addr.ToU64());
  }
}

void Elect(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data);
  if (r->state_ == FOLLOWER || r->state_ == CANDIDATE) {  // if necessary??
    r->meta_.IncrTerm();
    r->state_ = CANDIDATE;
    r->leader_ = 0;
    r->vote_mgr_.Clear();

    // vote for myself
    r->meta_.SetVote(r->Me().ToU64());

    // send request-vote
    for (auto &dest_addr : r->Peers()) {
      r->DoRequestVote(dest_addr.ToU64());
    }

    // reset election timer
    r->election_timer_->Again(r->random_election_ms_.Get(), 0);

    // only myself
    if (r->vote_mgr_.QuorumAll(r->VoteForMyself())) {
      r->BecomeLeader();
    }
  }
}

void HeartBeat(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data);
  if (r->state_ == LEADER) {  // if necessary??
    // send append-entries
    for (auto &dest_addr : r->Peers()) {
      r->DoAppendEntries(dest_addr.ToU64());
    }
  }
}

// if path is empty, use rc to initialize,
// else use the data in path to initialize
Raft::Raft(const std::string &path, const RaftConfig &rc)
    : home_path_(path),
      conf_path_(path + "/conf/conf.json"),
      meta_path_(path + "/meta"),
      log_path_(path + "/rlog"),
      sm_path_(path + "/sm"),
      meta_(path + "/meta"),
      log_(path + "/rlog"),
      config_mgr_(rc),
      vote_mgr_(rc.peers),
      index_mgr_(rc.peers),
      sm_(path + "/sm"),
      ping_timer_ms_(1000),
      election_timer_ms_(1500),
      heartbeat_timer_ms_(500),
      random_election_ms_(election_timer_ms_, 2 * election_timer_ms_),
      state_(FOLLOWER),
      commit_(0),
      last_apply_(0),
      leader_(0) {}

int32_t Raft::Start() {
  // user maybe set make_timer after Init()
  if (make_timer_) {
    ping_timer_ = make_timer_(0, ping_timer_ms_, PingPeers, this);
    election_timer_ = make_timer_(random_election_ms_.Get(), 0, Elect, this);
    heartbeat_timer_ = make_timer_(0, heartbeat_timer_ms_, HeartBeat, this);

    ping_timer_->Start();

  } else {
    return -1;
  }

  // become follower
  StepDown(meta_.term());

  return 0;
}

int32_t Raft::Stop() {
  ping_timer_->Stop();
  return 0;
}

void Raft::Init() {
  int32_t rv;
  char cmd_buf[256];

  snprintf(cmd_buf, sizeof(cmd_buf), "mkdir -p %s/conf", home_path_.c_str());
  rv = system(cmd_buf);
  assert(rv == 0);

  snprintf(cmd_buf, sizeof(cmd_buf), "mkdir -p %s", log_path_.c_str());
  system(cmd_buf);
  assert(rv == 0);

  snprintf(cmd_buf, sizeof(cmd_buf), "mkdir -p %s", meta_path_.c_str());
  system(cmd_buf);
  assert(rv == 0);

  rv = InitConfig();
  assert(rv == 0);

  meta_.Init();
  log_.Init();
  sm_.Init();

  // reset managers
  index_mgr_.ResetNext(LastIndex() + 1);
  index_mgr_.ResetMatch(0);
}

int32_t Raft::InitConfig() {
  std::ifstream read_file(conf_path_);
  if (read_file) {
    read_file.close();

    // load config
    std::ifstream json_file(conf_path_);
    nlohmann::json j;
    json_file >> j;
    json_file.close();

    RaftConfig rc;
    rc.me = RaftAddr(j["config_manager"]["me"][0]);
    for (auto &peer : j["config_manager"]["peers"]) {
      rc.peers.push_back(RaftAddr(peer[0]));
    }
    config_mgr_.SetCurrent(rc);

    // reset managers
    index_mgr_.Reset(rc.peers);
    vote_mgr_.Reset(rc.peers);

  } else {
    // write config
    std::ofstream write_file(conf_path_);
    write_file << config_mgr_.ToJsonString(false, false);
    write_file.close();
  }

  return 0;
}

RaftIndex Raft::LastIndex() {
  RaftIndex snapshot_last = sm_.LastIndex();
  RaftIndex log_last = log_.Last();
  RaftIndex last = std::min(snapshot_last, log_last);
  return last;
}

RaftTerm Raft::LastTerm() {
  RaftIndex snapshot_last = sm_.LastIndex();
  RaftIndex log_last = log_.Last();

  if (log_last >= snapshot_last) {  // log is newer
    if (log_last == 0) {            // no log, no snapshot
      return meta_.term();          // return current term

    } else {                                       // has log
      return log_.LastEntry()->append_entry.term;  // return last log term
    }

  } else {  // snapshot is newer
    return sm_.LastTerm();
  }
}

RaftIndex Raft::PreIndex() {}

RaftTerm Raft::PreTerm() {}

bool Raft::VoteForMyself() { return (meta_.vote() == Me().ToU64()); }

void Raft::StepDown(RaftTerm new_term) {
  assert(meta_.term() <= new_term);
  if (meta_.term() < new_term) {  // newer term
    meta_.SetTerm(new_term);
    meta_.SetVote(0);
    leader_ = 0;
    state_ = FOLLOWER;

  } else {  // equal term
    if (state_ != FOLLOWER) {
      state_ = FOLLOWER;
    }
  }

  // close heartbeat timer
  heartbeat_timer_->Stop();

  // start election timer
  election_timer_->Start();
}

void Raft::BecomeLeader() {
  assert(state_ == CANDIDATE);
  state_ = LEADER;
  leader_ = Me().ToU64();

  // stop election timer
  election_timer_->Stop();

  // reset index manager
  index_mgr_.ResetNext(LastIndex() + 1);
  index_mgr_.ResetMatch(0);

  // append noop

  // start heartbeat timer
  heartbeat_timer_->Start();
}

int32_t Raft::OnPing(struct Ping &msg) {
  Tracer tracer(this, false);
  tracer.PrepareState0();
  tracer.PrepareEvent(kRecv, msg.ToJsonString(false, true));

  PingReply reply;
  reply.src = msg.dest;
  reply.dest = msg.src;
  reply.msg = "pang";
  std::string reply_str;
  int32_t bytes = reply.ToString(reply_str);

  MsgHeader header;
  header.body_bytes = bytes;
  header.type = kPingReply;
  std::string header_str;
  header.ToString(header_str);

  if (send_) {
    header_str.append(std::move(reply_str));
    send_(reply.dest.ToU64(), header_str.data(), header_str.size());
    tracer.PrepareEvent(kSend, reply.ToJsonString(false, true));
  }

  tracer.PrepareState1();
  vraft_logger.Trace("%s", tracer.Finish().c_str());

  return 0;
}

int32_t Raft::OnPingReply(struct PingReply &msg) {
  vraft_logger.Info("%s recv ping-reply from %s, msg:%s",
                    msg.dest.ToString().c_str(), msg.src.ToString().c_str(),
                    msg.msg.c_str());
  return 0;
}

int32_t Raft::OnRequestVote(struct RequestVote &msg) { return 0; }

int32_t Raft::OnRequestVoteReply(struct RequestVoteReply &msg) { return 0; }

int32_t Raft::OnAppendEntries(struct AppendEntries &msg) { return 0; }

int32_t Raft::OnAppendEntriesReply(struct AppendEntriesReply &msg) { return 0; }

int32_t Raft::OnInstallSnapshot(struct InstallSnapshot &msg) { return 0; }

int32_t Raft::OnInstallSnapshotReply(struct InstallSnapshotReply &msg) {
  return 0;
}

int32_t Raft::DoPing(uint64_t dest) {
  Ping msg;
  msg.src = Me();
  msg.dest = RaftAddr(dest);
  msg.msg = "ping";

  std::string body_str;
  int32_t bytes = msg.ToString(body_str);

  MsgHeader header;
  header.body_bytes = bytes;
  header.type = kPing;
  std::string header_str;
  header.ToString(header_str);

  if (send_) {
    header_str.append(std::move(body_str));
    send_(dest, header_str.data(), header_str.size());
  }
  return 0;
}

int32_t Raft::DoRequestVote(uint64_t dest) {
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
  }
  return 0;
}

int32_t Raft::DoAppendEntries(uint64_t dest) {
  AppendEntries msg;
  msg.src = Me();
  msg.dest = RaftAddr(dest);
  msg.term = meta_.term();

  msg.pre_log_index = PreIndex();
  msg.pre_log_term = PreTerm();
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
  }
  return 0;
}

int32_t Raft::DoInstallSnapshot(uint64_t dest) { return 0; }

nlohmann::json Raft::ToJson() {
  nlohmann::json j;
  j["index"] = index_mgr_.ToJson();
  j["vote"] = vote_mgr_.ToJson();
  j["config"] = config_mgr_.ToJson();
  j["log"] = log_.ToJson();
  j["meta"] = meta_.ToJson();
  j["commit"] = commit_;
  j["last_apply"] = last_apply_;
  j["state"] = std::string(StateToStr(state_));
  if (leader_.ToU64() == 0) {
    j["leader"] = 0;
  } else {
    j["leader"] = leader_.ToString();
  }
  j["this"] = PointerToHexStr(this);
  return j;
}

nlohmann::json Raft::ToJsonTiny() {
  nlohmann::json j;
  for (auto dest : config_mgr_.Current().peers) {
    std::string key = "addr_";
    key.append(dest.ToString());
    j[key]["ma"] = index_mgr_.indices[dest.ToU64()].match;
    j[key]["ne"] = index_mgr_.indices[dest.ToU64()].next;
    j[key]["gr"] = vote_mgr_.votes[dest.ToU64()].grant;
    j[key]["dn"] = vote_mgr_.votes[dest.ToU64()].done;
  }

  j["log"] = log_.ToJsonTiny();
  j["tm"] = meta_.term();
  if (meta_.vote() == 0) {
    j["vt"] = "0";
  } else {
    RaftAddr addr(meta_.vote());
    j["vt"] = addr.ToString();
  }
  j["cmt"] = commit_;
  j["lapl"] = last_apply_;
  j["sta"] = std::string(StateToStr(state_));
  j["ts"] = PointerToHexStr(this);
  return j;
}

std::string Raft::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j[config_mgr_.Current().me.ToString()] = ToJsonTiny();
  } else {
    j[config_mgr_.Current().me.ToString()] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft
