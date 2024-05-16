#ifndef VRAFT_TCP_SERVER_H_
#define VRAFT_TCP_SERVER_H_

#include <map>
#include <memory>
#include <string>

#include "acceptor.h"
#include "eventloop.h"
#include "hostport.h"
#include "tcp_connection.h"
#include "tcp_options.h"
#include "timer.h"

namespace vraft {

class TcpServer;
using TcpServerPtr = std::shared_ptr<TcpServer>;

class TcpServer final {
 public:
  TcpServer(const HostPort &addr, const std::string &name,
            const TcpOptions &options, EventLoop *loop);
  ~TcpServer();
  TcpServer(const TcpServer &t) = delete;
  TcpServer &operator=(const TcpServer &t) = delete;

  // my thread is loop thread
  int32_t Start();

  // call in any thread
  int32_t Stop();
  int32_t AddConnection(TcpConnectionPtr &conn);
  int32_t RemoveConnection(const TcpConnectionPtr &conn);
  void RunFunctor(const Functor func);

  // call in loop thread
  bool IsStart();
  void set_on_connection_cb(const OnConnectionCallback &cb);
  void set_write_complete_cb(const WriteCompleteCallback &cb);
  void set_on_message_cb(const OnMessageCallback &cb);
  const std::string &name() const;

  HostPort Addr() const { return acceptor_.addr(); }

 private:
  // call in loop thread
  void Init();
  int32_t StopInLoop();

  void NewConnection(UvTcpUPtr client);
  int32_t AddConnectionInLoop(TcpConnectionPtr &conn);
  int32_t RemoveConnectionInLoop(const TcpConnectionPtr &conn);

 private:
  const std::string name_;

  EventLoop *loop_;
  ConnectionMap connections_;
  Acceptor acceptor_;

  OnMessageCallback on_message_cb_;
  OnConnectionCallback on_connection_cb_;
  WriteCompleteCallback write_complete_cb_;
};

// call in loop thread
inline bool TcpServer::IsStart() { return acceptor_.IsStart(); }

inline void TcpServer::set_on_connection_cb(const OnConnectionCallback &cb) {
  on_connection_cb_ = cb;
}

inline void TcpServer::set_write_complete_cb(const WriteCompleteCallback &cb) {
  write_complete_cb_ = cb;
}

inline void TcpServer::set_on_message_cb(const OnMessageCallback &cb) {
  on_message_cb_ = cb;
}

inline const std::string &TcpServer::name() const { return name_; }

}  // namespace vraft

#endif
