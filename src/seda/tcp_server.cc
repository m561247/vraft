#include "tcp_server.h"
#include "eventloop.h"
#include "vraft_logger.h"
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace vraft {

TcpServer::TcpServer(const HostPort &addr, const std::string &name,
                     const TcpOptions &options, EventLoop *loop)
    : name_(name), loop_(loop), connections_(), acceptor_(addr, loop, options) {
  vraft_logger.FInfo("tcp-server:%s construct", name_.c_str());
  // make sure the all the objects finish construct
  Init();
}

TcpServer::~TcpServer() {
  vraft_logger.FInfo("tcp-server:%s destruct", name_.c_str());
  // acceptor and connections will destroy themselves
  // Stop();
}

int32_t TcpServer::Start() {
  vraft_logger.FInfo("tcp-server:%s start", name_.c_str());
  int32_t r = acceptor_.Start();
  assert(r == 0);
  return r;
}

int32_t TcpServer::Stop() {
  loop_->RunFunctor(std::bind(&TcpServer::StopInLoop, this));
  return 0;
}

int32_t TcpServer::AddConnection(TcpConnectionPtr &conn) {
  int32_t r = 0;
  if (conn->loop()->IsInLoopThread()) {
    r = AddConnectionInLoop(conn);
  } else {
    conn->loop()->RunFunctor(
        std::bind(&TcpServer::AddConnectionInLoop, this, conn));
  }
  return r;
}

int32_t TcpServer::RemoveConnection(const TcpConnectionPtr &conn) {
  int32_t r = 0;
  if (conn->loop()->IsInLoopThread()) {
    r = RemoveConnectionInLoop(conn);
  } else {
    conn->loop()->RunFunctor(
        std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
  }
  return r;
}

// call in any thread
void TcpServer::RunFunctor(const Functor func) { loop_->RunFunctor(func); }

int32_t TcpServer::StopInLoop() {
  vraft_logger.FInfo("tcp-server:%s stop", name_.c_str());
  loop_->AssertInLoopThread();
  int32_t r = 0;

  ConnectionMap tmp_conns;
  tmp_conns.swap(connections_);
  for (auto conn_pair : tmp_conns) {
    r = conn_pair.second->Stop();
    assert(r == 0);
  }

  r = acceptor_.Stop();
  assert(r == 0);

  return 0;
}

void TcpServer::NewConnection(UvTcpUPtr client) {
  loop_->AssertInLoopThread();

  sockaddr_in local_addr, peer_addr;
  int namelen = sizeof(sockaddr_in);
  int rv = 0;

  rv = UvTcpGetSockName(client.get(), (struct sockaddr *)(&local_addr),
                        &namelen);
  assert(rv == 0);
  HostPort local_hp = SockaddrInToHostPort(&local_addr);

  rv =
      UvTcpGetPeerName(client.get(), (struct sockaddr *)(&peer_addr), &namelen);
  assert(rv == 0);
  HostPort peer_hp = SockaddrInToHostPort(&peer_addr);

  std::string conn_name =
      name_ + "#" + local_hp.ToString() + "#" + peer_hp.ToString();
  TcpConnectionPtr conn(new TcpConnection(loop_, conn_name, std::move(client)));

  conn->set_on_connection_cb(on_connection_cb_);
  conn->set_on_message_cb(on_message_cb_);
  conn->set_write_complete_cb(write_complete_cb_);

  // FIXME: should first close, then delete in callback
  conn->set_connection_close_cb(
      std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));

  AddConnection(conn);
  if (on_connection_cb_) {
    on_connection_cb_(conn);
  }
}

void TcpServer::Init() {
  acceptor_.set_new_conn_func(
      std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
}

int32_t TcpServer::AddConnectionInLoop(TcpConnectionPtr &conn) {
  conn->loop()->AssertInLoopThread();

  vraft_logger.FInfo("tcp-server:%s add connection:%s", name_.c_str(),
                     conn->name().c_str());
  connections_[conn->name()] = conn;
  int32_t r = conn->Start();
  return r;
}

int32_t TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn) {
  conn->loop()->AssertInLoopThread();

  vraft_logger.FInfo("tcp-server:%s remove connection:%s", name_.c_str(),
                     conn->name().c_str());
  size_t n = connections_.erase(conn->name());
  assert(n == 1);
  return 0;
}

} // namespace vraft
