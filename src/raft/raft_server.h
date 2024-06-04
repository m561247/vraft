#ifndef VRAFT_RAFT_SERVER_H_
#define VRAFT_RAFT_SERVER_H_

#include <memory>
#include <unordered_map>

#include "allocator.h"
#include "config.h"
#include "raft.h"
#include "tcp_client.h"
#include "tcp_server.h"

namespace vraft {

class RaftServer;
using RaftServerPtr = std::shared_ptr<RaftServer>;

class Remu;
using RemuPtr = std::shared_ptr<Remu>;

// raft emulator, good!!
struct Remu {
  Remu(EventLoopSPtr l) : loop(l) {}

  EventLoopWPtr loop;
  std::vector<vraft::Config> configs;
  std::vector<vraft::RaftServerPtr> raft_servers;

  void Create();
  void Start();
  void Stop();
  void Clear();
  void Print(bool tiny = true, bool one_line = true);
};

class RaftServer final {
 public:
  RaftServer(Config &config, EventLoopSPtr loop);
  ~RaftServer();
  RaftServer(const RaftServer &t) = delete;
  RaftServer &operator=(const RaftServer &t) = delete;

  void OnConnection(const vraft::TcpConnectionSPtr &conn);
  void OnMessage(const vraft::TcpConnectionSPtr &conn, vraft::Buffer *buf);

  int32_t Start();
  int32_t Stop();

  Config &config() { return config_; }
  EventLoopSPtr LoopSPtr() {
    auto sptr = loop_.lock();
    return sptr;
  }
  RaftPtr raft() { return raft_; }

  void Print(bool tiny, bool one_line) { raft_->Print(tiny, one_line); }

 private:
  // for raft func
  TcpClientPtr GetClient(uint64_t dest_addr);
  TcpClientPtr GetClientOrCreate(uint64_t dest_addr);
  int32_t Send(uint64_t dest_addr, const char *buf, unsigned int size);
  TimerSPtr MakeTimer(TimerParam &param);

 private:
  Config config_;
  EventLoopWPtr loop_;
  TcpServerSPtr server_;
  std::unordered_map<uint64_t, TcpClientPtr> clients_;

  RaftPtr raft_;
};

// FIXME: raft_(this), maybe this has not finished construct
inline RaftServer::RaftServer(Config &config, EventLoopSPtr loop)
    : config_(config), loop_(loop) {
  struct TcpOptions options;

  auto sptr = loop_.lock();
  assert(sptr);
  server_ = std::make_shared<TcpServer>(sptr, config_.my_addr(), "raft_server",
                                        options);

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
  raft_ = std::make_shared<Raft>(config_.path(), rc);
  raft_->Init();
  raft_->set_send(std::bind(&RaftServer::Send, this, std::placeholders::_1,
                            std::placeholders::_2, std::placeholders::_3));
  raft_->set_make_timer(
      std::bind(&RaftServer::MakeTimer, this, std::placeholders::_1));
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
    auto sptr = loop_.lock();
    assert(sptr);
    ptr = std::make_shared<TcpClient>("raft_client", sptr, hostport, options);
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
