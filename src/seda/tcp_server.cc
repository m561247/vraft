#include "tcp_server.h"

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "eventloop.h"
#include "vraft_logger.h"

namespace vraft {

TcpServer::TcpServer(const HostPort &addr, const std::string &name,
                     const TcpOptions &options, EventLoopSPtr loop)
    : name_(name), loop_(loop), connections_(), acceptor_(loop, addr, options) {
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
  auto sptr = loop_.lock();
  if (sptr) {
    sptr->RunFunctor(std::bind(&TcpServer::StopInLoop, this));
  }

  return 0;
}

void TcpServer::AddConnection(TcpConnectionSPtr &conn) {
  auto lptr = conn->LoopSPtr();
  if (lptr) {
    if (lptr->IsInLoopThread()) {
      AddConnectionInLoop(conn);
    } else {
      lptr->RunFunctor(std::bind(&TcpServer::AddConnectionInLoop, this, conn));
    }
  }
}

void TcpServer::RemoveConnection(const TcpConnectionSPtr &conn) {
  auto lptr = conn->LoopSPtr();
  if (lptr) {
    if (lptr->IsInLoopThread()) {
      RemoveConnectionInLoop(conn);
    } else {
      lptr->RunFunctor(
          std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
    }
  }
}

// call in any thread
void TcpServer::RunFunctor(const Functor func) {
  auto sptr = loop_.lock();
  if (sptr) {
    sptr->RunFunctor(func);
  }
}

void TcpServer::AssertInLoopThread() {
  auto sptr = loop_.lock();
  if (sptr) {
    sptr->AssertInLoopThread();
  }
}

int32_t TcpServer::StopInLoop() {
  vraft_logger.FInfo("tcp-server:%s stop", name_.c_str());
  AssertInLoopThread();
  int32_t r = 0;

  ConnectionMap tmp_conns = connections_;
  for (auto conn_pair : tmp_conns) {
    r = conn_pair.second->Close();
    assert(r == 0);
  }
  tmp_conns.clear();

  r = acceptor_.Close();
  assert(r == 0);

  return 0;
}

void TcpServer::NewConnection(UvTcpUPtr client) {
  AssertInLoopThread();

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

  auto sptr = loop_.lock();
  assert(sptr);

  TcpConnectionSPtr conn(new TcpConnection(sptr, conn_name, std::move(client)));

  conn->set_on_connection_cb(on_connection_cb_);
  conn->set_on_message_cb(on_message_cb_);
  conn->set_write_complete_cb(write_complete_cb_);
  conn->set_connection_close_cb(
      std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
  vraft_logger.FInfo("tcp-server:%s new connection:%s", name_.c_str(),
                     conn->ToString().c_str());

  AddConnection(conn);
  if (on_connection_cb_) {
    on_connection_cb_(conn);
  }
}

void TcpServer::Init() {
  acceptor_.set_new_conn_func(
      std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
}

int32_t TcpServer::AddConnectionInLoop(TcpConnectionSPtr &conn) {
  AssertInLoopThread();

  vraft_logger.FInfo("tcp-server:%s add connection:%s", name_.c_str(),
                     conn->name().c_str());
  connections_[conn->name()] = conn;
  int32_t r = conn->Start();
  return r;
}

int32_t TcpServer::RemoveConnectionInLoop(const TcpConnectionSPtr &conn) {
  AssertInLoopThread();

  vraft_logger.FInfo("tcp-server:%s remove connection:%s", name_.c_str(),
                     conn->name().c_str());
  size_t n = connections_.erase(conn->name());
  assert(n == 1);
  return 0;
}

}  // namespace vraft
