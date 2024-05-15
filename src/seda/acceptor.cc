#include "acceptor.h"
#include "eventloop.h"
#include "uv_wrapper.h"
#include "vraft_logger.h"
#include <cassert>
#include <memory>

namespace vraft {

Acceptor::Acceptor(const HostPort &addr, EventLoop *loop,
                   const TcpOptions &options)
    : addr_(addr), options_(options), loop_(loop) {
  vraft_logger.FInfo("acceptor:%s construct, handle:%p",
                     addr_.ToString().c_str(), &server_);
  Init();
}

Acceptor::~Acceptor() {
  vraft_logger.FInfo("acceptor:%s destruct, handle:%p",
                     addr_.ToString().c_str(), &server_);
  Stop();
}

void Acceptor::AssertInLoopThread() const { loop_->AssertInLoopThread(); }

int32_t Acceptor::Start() {
  // loop not start
  // loop_->AssertInLoopThread();
  vraft_logger.FInfo("acceptor:%s start, handle:%p", addr_.ToString().c_str(),
                     &server_);

  int32_t r = Bind();
  assert(r == 0);
  r = Listen();
  assert(r == 0);
  return 0;
}

int32_t Acceptor::Stop() {
  loop_->AssertInLoopThread();
  vraft_logger.FInfo("acceptor:%s stop, handle:%p", addr_.ToString().c_str(),
                     &server_);

  if (!UvIsClosing(reinterpret_cast<UvHandle *>(&server_))) {
    UvClose(reinterpret_cast<UvHandle *>(&server_), nullptr);
  } else {
    vraft_logger.FError("acceptor:%s stop, already stopping, handle:%p",
                        addr_.ToString().c_str(), &server_);
  }

  return 0;
}

// Acceptor::NewConnection is a function broker
// called by AcceptorHandleRead
// to call TcpServer::NewConnection
void Acceptor::NewConnection(UvTcpUPtr conn) {
  loop_->AssertInLoopThread();

  assert(new_conn_func_);
  new_conn_func_(std::move(conn));
}

bool Acceptor::IsStart() {
  return UvIsActive(reinterpret_cast<UvHandle *>(&server_));
}

int32_t Acceptor::Bind() {
  // loop not start
  // loop_unit_->AssertInLoopThread();

  int32_t r = UvTcpBind(&server_, &(addr_.addr), 0);
  return r;
}

int32_t Acceptor::Listen() {
  // loop not start
  // loop_unit_->AssertInLoopThread();

  int32_t r = UvListen((UvStream *)&server_, 128, AcceptorHandleRead);
  return r;
}

void Acceptor::Init() {
  UvTcpInit(loop_->UvLoopPtr(), &server_);
  if (options_.tcp_nodelay) {
    UvTcpNodelay(&server_, 1);
  }
  server_.data = this;
}

void AcceptorHandleRead(UvStream *server, int status) {
  Acceptor *acceptor = static_cast<Acceptor *>(server->data);
  acceptor->loop()->AssertInLoopThread();
  assert(status == 0);

  UvTcpUPtr client = std::make_unique<UvTcp>();
  int32_t r = UvTcpInit(acceptor->loop()->UvLoopPtr(), client.get());
  assert(r == 0);

  if (acceptor->options().tcp_nodelay) {
    UvTcpNodelay(client.get(), 1);
  }

  if (UvAccept(server, (UvStream *)client.get()) == 0) {
    acceptor->NewConnection(std::move(client));

  } else {
    // UvTcpUPtr will free UvTcp, so need not OnClose
    UvClose(reinterpret_cast<UvHandle *>(client.get()), nullptr);
  }
}

} // namespace vraft
