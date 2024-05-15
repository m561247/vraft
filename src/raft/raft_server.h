#ifndef VRAFT_RAFT_SERVER_H_
#define VRAFT_RAFT_SERVER_H_

#include "allocator.h"
#include "config.h"
#include "raft.h"
#include "tcp_client.h"
#include "tcp_server.h"
#include <unordered_map>

namespace vraft {

class RaftServer final {
public:
  RaftServer(Config &config);
  ~RaftServer();
  RaftServer(const RaftServer &t) = delete;
  RaftServer &operator=(const RaftServer &t) = delete;

  void OnConnection(const vraft::TcpConnectionPtr &conn);
  void OnMessage(const vraft::TcpConnectionPtr &conn, vraft::Buffer *buf);

  int32_t Start();
  int32_t Stop();
  TcpClientPtr GetClient(uint64_t dest_addr);
  TcpClientPtr GetClientOrCreate(uint64_t dest_addr);

  Config &config() { return config_; }
  EventLoop *LoopPtr() { return &loop_; }

private:
  Config config_;
  EventLoop loop_;
  TcpServerPtr server_;
  std::unordered_map<uint64_t, TcpClientPtr> clients_;
  Raft raft_;
};

// FIXME: raft_(this), maybe this has not finished construct
inline RaftServer::RaftServer(Config &config)
    : loop_("single_raft_loop"), config_(config), raft_(this) {
  struct TcpOptions options;
  server_ = std::make_shared<TcpServer>(config_.my_addr(), "raft_server",
                                        options, &loop_);

  server_->set_on_connection_cb(
      std::bind(&RaftServer::OnConnection, this, std::placeholders::_1));
  server_->set_on_message_cb(std::bind(&RaftServer::OnMessage, this,
                                       std::placeholders::_1,
                                       std::placeholders::_2));

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
    ptr = std::make_shared<TcpClient>("raft_client", &loop_, hostport, options);
    int32_t rv = ptr->Connect(100);
    if (rv == 0) {
      clients_.insert({dest_addr, ptr});
    } else {
      ptr.reset();
    }
  }

  return ptr;
}

} // namespace vraft

#endif
