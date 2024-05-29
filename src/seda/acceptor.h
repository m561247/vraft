#ifndef VRAFT_ACCEPTOR_H_
#define VRAFT_ACCEPTOR_H_

#include <atomic>

#include "common.h"
#include "hostport.h"
#include "tcp_connection.h"
#include "tcp_options.h"

namespace vraft {

class EventLoop;
using AcceptorNewConnFunc = std::function<void(UvTcpUPtr)>;

void AcceptorHandleRead(UvStream *server, int status);

class Acceptor final {
 public:
  Acceptor(const HostPort &addr, EventLoop *loop, const TcpOptions &options);
  ~Acceptor();
  Acceptor(const Acceptor &t) = delete;
  Acceptor &operator=(const Acceptor &t) = delete;

  // call in loop thread
  int32_t Start();
  int32_t Close();
  bool IsStart();
  void NewConnection(UvTcpUPtr conn);
  void AssertInLoopThread() const;

  // call in loop thread
  const HostPort &addr() const;
  const TcpOptions &options() const;
  EventLoop *loop() const;
  void set_new_conn_func(const AcceptorNewConnFunc &new_conn_func);

 private:
  // call in loop thread
  void Init();
  int32_t Bind();
  int32_t Listen();

 private:
  const HostPort addr_;
  const TcpOptions options_;
  EventLoop *loop_;
  UvTcp server_;

  AcceptorNewConnFunc new_conn_func_;
};

inline const HostPort &Acceptor::addr() const { return addr_; }

inline const TcpOptions &Acceptor::options() const { return options_; }

inline EventLoop *Acceptor::loop() const { return loop_; }

inline void Acceptor::set_new_conn_func(
    const AcceptorNewConnFunc &new_conn_func) {
  new_conn_func_ = new_conn_func;
}

}  // namespace vraft

#endif
