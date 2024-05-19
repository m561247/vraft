#include "raft.h"

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

void PingTimerFunc(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data);
  for (auto &dest_addr : r->Peers()) {
    Ping msg;
    msg.msg = "ping";

    uint32_t ip32;
    bool b = false;

    msg.src = r->Me();
    msg.dest = dest_addr;

    std::string msg_str;
    int32_t bytes = msg.ToString(msg_str);

    MsgHeader header;
    header.body_bytes = bytes;
    header.type = kMsgPing;
    std::string header_str;
    header.ToString(header_str);

    if (r->send_) {
      header_str.append(std::move(msg_str));
      r->send_(msg.dest.ToU64(), header_str.data(), header_str.size());
    }
  }
}

void ElectionTimerFunc(Timer *timer) {}

void HeartbeatTimerFunc(Timer *timer) {}

// if path is empty, use rc to initialize,
// else use the data in path to initialize
Raft::Raft(const std::string &path, const RaftConfig &rc)
    : home_path_(path),
      conf_path_(path + "/conf/conf.json"),
      meta_path_(path + "/meta"),
      log_path_(path + "/raft_log"),
      meta_(path + "/meta"),
      log_(path + "/raft_log"),
      config_mgr_(rc),
      vote_mgr_(rc.peers),
      index_mgr_(rc.peers),
      ping_timer_ms_(1000),
      election_timer_ms_(1500),
      heartbeat_timer_ms_(500),
      random_election_ms_(election_timer_ms_, 2 * election_timer_ms_),
      state_(FOLLOWER),
      commit_(0),
      leader_(0) {}

int32_t Raft::Start() {
  if (make_timer_) {
    ping_timer_ = make_timer_(0, ping_timer_ms_, PingTimerFunc, this);
    election_timer_ =
        make_timer_(0, random_election_ms_.Get(), ElectionTimerFunc, this);
    heartbeat_timer_ =
        make_timer_(0, heartbeat_timer_ms_, HeartbeatTimerFunc, this);

  } else {
    return -1;
  }

  ping_timer_->Start();
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
    rc.me = RaftAddr(j["config_manager"]["me"]["u64"]);
    for (auto &peer : j["config_manager"]["peers"]) {
      rc.peers.push_back(RaftAddr(peer["u64"]));
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
  header.type = kMsgPingReply;
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

nlohmann::json Raft::ToJson() {
  nlohmann::json j;
  j["index"] = index_mgr_.ToJson();
  j["vote"] = vote_mgr_.ToJson();
  j["config"] = config_mgr_.ToJson();
  j["log"] = log_.ToJson();
  j["meta"] = meta_.ToJson();
  j["commit"] = commit_;
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
    j[key]["m"] = index_mgr_.indices[dest.ToU64()].match;
    j[key]["n"] = index_mgr_.indices[dest.ToU64()].next;
    j[key]["v"] = vote_mgr_.votes[dest.ToU64()];
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
  j["this"] = PointerToHexStr(this);
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
