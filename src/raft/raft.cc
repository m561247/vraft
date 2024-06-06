#include "raft.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>

#include "clock.h"
#include "raft_server.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

const char *StateToStr(enum State state) {
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

void Tick(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data());
  vraft_logger.FInfo("raft-tick: %s", r->ToJsonString(true, true).c_str());
  for (auto &dest_addr : r->Peers()) {
    r->SendPing(dest_addr.ToU64());
  }
}

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

// if path is empty, use rc to initialize,
// else use the data in path to initialize
Raft::Raft(const std::string &path, const RaftConfig &rc)
    : home_path_(path),
      conf_path_(path + "/conf/conf.json"),
      meta_path_(path + "/meta"),
      log_path_(path + "/rlog"),
      sm_path_(path + "/sm"),
      state_(FOLLOWER),
      commit_(0),
      last_apply_(0),
      leader_(0),
      started_(false),
      meta_(path + "/meta"),
      log_(path + "/rlog"),
      config_mgr_(rc),
      index_mgr_(rc.peers),
      vote_mgr_(rc.peers),
      timer_mgr_(rc.peers),
      sm_(path + "/sm") {
  vraft_logger.FInfo("raft construct, %s, %p", rc.me.ToString().c_str(), this);
}

Raft::~Raft() { vraft_logger.FInfo("raft destruct, %p", this); }

int32_t Raft::Start() {
  started_ = true;

  // make timer
  assert(make_timer_);
  timer_mgr_.set_maketimer_func(make_timer_);
  timer_mgr_.set_tick_func(Tick);
  timer_mgr_.set_election_func(Elect);
  timer_mgr_.set_requestvote_func(RequestVoteRpc);
  timer_mgr_.set_heartbeat_func(HeartBeat);
  timer_mgr_.set_data(this);
  timer_mgr_.MakeTimer();

  // start tick
  timer_mgr_.StartTick();

  // become follower
  StepDown(meta_.term());

  return 0;
}

int32_t Raft::Stop() {
  started_ = false;
  timer_mgr_.Close();
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
    timer_mgr_.Reset(rc.peers);

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

    } else {                         // has log
      return log_.LastMeta()->term;  // return last log term
    }

  } else {  // snapshot is newer
    return sm_.LastTerm();
  }
}

RaftTerm Raft::GetTerm(RaftIndex index) {
  assert(index >= 0);
  if (index == 0) {
    return 0;
  }

  assert(index >= 1);
  MetaValue meta;
  int32_t rv = log_.GetMeta(index, meta);
  if (rv == 0) {
    return meta.term;
  } else {
    return sm_.LastTerm();
  }
}

bool Raft::IfSelfVote() { return (meta_.vote() == Me().ToU64()); }

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

void Raft::AppendNoop() {
  AppendEntry entry;
  entry.term = meta_.term();
  entry.type = kNoop;
  entry.value.append("0");
  int32_t rv = log_.AppendOne(entry);
  assert(rv == 0);
}

void Raft::MaybeCommit() {}

int32_t Raft::SendPing(uint64_t dest) {
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

int32_t Raft::SendRequestVoteReply(RequestVoteReply &msg) {
  std::string body_str;
  int32_t bytes = msg.ToString(body_str);

  MsgHeader header;
  header.body_bytes = bytes;
  header.type = kRequestVoteReply;
  std::string header_str;
  header.ToString(header_str);

  if (send_) {
    header_str.append(std::move(body_str));
    send_(msg.dest.ToU64(), header_str.data(), header_str.size());
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

int32_t Raft::SendInstallSnapshotReply(InstallSnapshotReply &msg) { return 0; }

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
  j["run"] = started_;
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

  j[0][0]["term"] = meta_.term();
  if (meta_.vote() == 0) {
    j[0][0]["vote"] = "0";
  } else {
    RaftAddr addr(meta_.vote());
    j[0][0]["vote"] = addr.ToString();
  }
  j[0][1]["log"] = log_.ToJsonTiny();

  j[0][2]["apply"] = last_apply_;
  j[0][2]["cmt"] = commit_;
  j[0][2]["leader"] = leader_.ToString();
  j[0][2]["run"] = started_;

  for (auto dest : config_mgr_.Current().peers) {
    std::string key;
    key.append(dest.ToString());
    j[0][3][key]["match"] = index_mgr_.indices[dest.ToU64()].match;
    j[0][3][key]["next"] = index_mgr_.indices[dest.ToU64()].next;
    j[0][3][key]["grant"] = vote_mgr_.votes[dest.ToU64()].grant;
    // j[key]["dn"] = vote_mgr_.votes[dest.ToU64()].done;
  }

  j[1] = PointerToHexStr(this);
  return j;
}

std::string Raft::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j[config_mgr_.Current().me.ToString()][0] = std::string(StateToStr(state_));
    j[config_mgr_.Current().me.ToString()][1] = ToJsonTiny();
  } else {
    j[config_mgr_.Current().me.ToString()][0] = std::string(StateToStr(state_));
    j[config_mgr_.Current().me.ToString()][1] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

void Raft::Print(bool tiny, bool one_line) {
  printf("%s\n", ToJsonString(tiny, one_line).c_str());
  fflush(nullptr);
}

}  // namespace vraft
