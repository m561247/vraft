#include "raft.h"
#include "raft_server.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

void SendPing(Timer *timer) {
  Raft *r = reinterpret_cast<Raft *>(timer->data);
  for (auto &hostport : r->peers_) {
    Ping msg;
    msg.msg = "ping";

    uint32_t ip32;
    bool b = false;

    b = StringToIpU32(r->my_addr_.host, ip32);
    assert(b);
    RaftAddr src_addr(ip32, r->my_addr_.port, r->id_);
    msg.src = src_addr;

    b = StringToIpU32(hostport.host, ip32);
    assert(b);
    RaftAddr dest_addr(ip32, hostport.port, r->id_);
    msg.dest = dest_addr;

    std::string msg_str;
    int32_t bytes = msg.ToString(msg_str);

    MsgHeader header;
    header.body_bytes = bytes;
    header.type = kMsgPing;
    std::string header_str;
    header.ToString(header_str);

    TcpClientPtr client = r->raft_server_->GetClientOrCreate(msg.dest.ToU64());
    if (client) {
      header_str.append(std::move(msg_str));
      client->CopySend(header_str.data(), header_str.size());
    } else {
      vraft_logger.FError("GetClientOrCreate error");
    }
  }
}

Raft::Raft(RaftServer *raft_server) : raft_server_(raft_server), id_(0) {
  my_addr_ = raft_server_->config().my_addr();
  for (auto peer : raft_server_->config().peers()) {
    peers_.push_back(peer);
  }

  ping_timer_ = raft_server_->LoopPtr()->MakeTimer(0, 1000, SendPing, this);
}

int32_t Raft::Start() {
  ping_timer_->Start();
  return 0;
}

int32_t Raft::Stop() {
  ping_timer_->Stop();
  return 0;
}

int32_t Raft::OnPing(struct Ping &msg) {
  vraft_logger.Info("%s recv ping from %s, msg:%s", msg.dest.ToString().c_str(),
                    msg.src.ToString().c_str(), msg.msg.c_str());

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

  TcpClientPtr client = raft_server_->GetClientOrCreate(reply.dest.ToU64());
  if (client) {
    header_str.append(std::move(reply_str));
    client->CopySend(header_str.data(), header_str.size());
  } else {
    vraft_logger.FError("GetClientOrCreate error");
  }

  return 0;
}

int32_t Raft::OnPingReply(struct PingReply &msg) {
  vraft_logger.Info("%s recv ping-reply from %s, msg:%s",
                    msg.dest.ToString().c_str(), msg.src.ToString().c_str(),
                    msg.msg.c_str());
  return 0;
}

} // namespace vraft
