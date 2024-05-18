#include "raft.h"

#include <cstdlib>

#include "clock.h"
#include "raft_server.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

void SendPing(Timer *timer) {
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

// if path is empty, use rc to initialize,
// else use the data in path to initialize
Raft::Raft(const std::string &path, const RaftConfig &rc)
    : home_path_(path),
      conf_path_(path + "/conf"),
      meta_path_(path + "/meta"),
      log_path_(path + "/raft_log"),
      meta_(path + "/meta"),
      log_(path + "/raft_log"),
      config_mgr_(rc),
      vote_mgr_(rc.peers),
      index_mgr_(rc.peers),
      tracer(this, true),
      commit_(0),
      leader_(0) {}

int32_t Raft::Start() {
  if (make_timer_) {
    ping_timer_ = make_timer_(0, 1000, SendPing, this);

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

  snprintf(cmd_buf, sizeof(cmd_buf), "mkdir -p %s/raft_log",
           home_path_.c_str());
  system(cmd_buf);
  assert(rv == 0);

  snprintf(cmd_buf, sizeof(cmd_buf), "mkdir -p %s/meta", home_path_.c_str());
  system(cmd_buf);
  assert(rv == 0);
}

int32_t Raft::OnPing(struct Ping &msg) {
  uint64_t ts = Clock::NSec();
  std::string trace_log;
  char trace_log_buf[256];

  snprintf(trace_log_buf, sizeof(trace_log_buf), "\n%llu state_0: ", ts);
  trace_log.append(trace_log_buf);
  trace_log.append(ToJsonString(true, true));

  snprintf(trace_log_buf, sizeof(trace_log_buf), "\n%llu event_r: ", ts);
  trace_log.append(trace_log_buf);
  trace_log.append(msg.ToJsonString(false, true));

  snprintf(trace_log_buf, sizeof(trace_log_buf), "\n%llu state_1: ", ts);
  trace_log.append(trace_log_buf);
  trace_log.append(ToJsonString(true, true));

  vraft_logger.Info("%s", trace_log.c_str());

  // vraft_logger.Info("%s recv ping from %s, msg:%s",
  // msg.dest.ToString().c_str(),
  //                  msg.src.ToString().c_str(), msg.msg.c_str());

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
    send_(msg.dest.ToU64(), header_str.data(), header_str.size());
  }

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
  j["me"] = config_mgr_.Current().me.ToString();
  for (auto dest : config_mgr_.Current().peers) {
    std::string key = "peer_";
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
    j["raft"] = ToJsonTiny();
  } else {
    j["raft"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft
