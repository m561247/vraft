#ifndef VRAFT_RAFT_SERVER_H_
#define VRAFT_RAFT_SERVER_H_

#include <unordered_map>

#include "allocator.h"
#include "config.h"
#include "raft.h"
#include "tcp_client.h"
#include "tcp_server.h"

namespace vraft {

class RaftServer final {
 public:
  RaftServer(Config &config, EventLoop *loop);
  ~RaftServer();
  RaftServer(const RaftServer &t) = delete;
  RaftServer &operator=(const RaftServer &t) = delete;

  void OnConnection(const vraft::TcpConnectionPtr &conn);
  void OnMessage(const vraft::TcpConnectionPtr &conn, vraft::Buffer *buf);

  int32_t Start();
  int32_t Stop();

  Config &config() { return config_; }
  EventLoop *LoopPtr() { return loop_; }

 private:
  // for raft func
  TcpClientPtr GetClient(uint64_t dest_addr);
  TcpClientPtr GetClientOrCreate(uint64_t dest_addr);
  int32_t SendMsg(uint64_t dest_addr, const char *buf, unsigned int size);
  TimerPtr MakeTimer(uint64_t timeout_ms, uint64_t repeat_ms,
                     const TimerFunctor &func, void *data);

 private:
  Config config_;
  EventLoop *loop_;
  TcpServerPtr server_;
  std::unordered_map<uint64_t, TcpClientPtr> clients_;

  RaftUPtr raft_;
};

// FIXME: raft_(this), maybe this has not finished construct
inline RaftServer::RaftServer(Config &config, EventLoop *loop)
    : loop_(loop), config_(config) {
  struct TcpOptions options;
  server_ = std::make_shared<TcpServer>(config_.my_addr(), "raft_server",
                                        options, loop_);

  server_->set_on_connection_cb(
      std::bind(&RaftServer::OnConnection, this, std::placeholders::_1));
  server_->set_on_message_cb(std::bind(&RaftServer::OnMessage, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2));

  RaftConfig rc;
  RaftAddr me(config_.my_addr().ip32, config_.my_addr().port, 0);
  rc.me = me;
  for (auto hostport : config_.peers()) {
    RaftAddr dest(hostport.ip32, hostport.port, 0);
    rc.peers.push_back(dest);
  }
  raft_ = std::make_unique<Raft>(config_.path(), rc);
  raft_->Init();
  raft_->set_send(std::bind(&RaftServer::SendMsg, this, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3));
  raft_->set_make_timer(std::bind(
      &RaftServer::MakeTimer, this, std::placeholders::_1,
      std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));

#if 0
  for (const auto &hostport : config_.peers()) {
    TcpClientPtr ptr =
        std::make_shared<TcpClient>("raft_client", &loop_, hostport, options);

    uint32_t ip;
    bool b = StringToIpU32(hostport.host, ip);
    assert(b);
    RaftAddr dest_addr(ip, hostport.port, 0);
    clients_.insert({dest_addr.ToU64(), ptr});
  }
#endif
}

inline RaftServer::~RaftServer() {}

inline TcpClientPtr RaftServer::GetClient(uint64_t dest_addr) {
  auto it = clients_.find(dest_addr);
  if (it != clients_.end()) {
    return it->second;

  } else {
    // not found
    return nullptr;
  }
}

inline TcpClientPtr RaftServer::GetClientOrCreate(uint64_t dest_addr) {
  TcpClientPtr ptr = GetClient(dest_addr);
  if (ptr && !ptr->Connected()) {
    clients_.erase(dest_addr);
    ptr.reset();
  }

  if (!ptr) {
    struct TcpOptions options;
    RaftAddr addr(dest_addr);
    HostPort hostport(IpU32ToIpString(addr.ip()), addr.port());
    ptr = std::make_shared<TcpClient>("raft_client", loop_, hostport, options);
    int32_t rv = ptr->Connect(100);
    if (rv == 0) {
      clients_.insert({dest_addr, ptr});
    } else {
      ptr.reset();
    }
  }

  return ptr;
}

}  // namespace vraft

#endif
