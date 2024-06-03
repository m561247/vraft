#ifndef VRAFT_CONNECTOR_H_
#define VRAFT_CONNECTOR_H_

#include "common.h"
#include "eventloop.h"
#include "hostport.h"
#include "tcp_connection.h"
#include "tcp_options.h"
#include "uv_wrapper.h"

namespace vraft {

using ConnectorNewConnFunc = std::function<void(UvTcpUPtr)>;

void ConnectFinish(UvConnect *req, int32_t status);
void TimerConnectCb(Timer *timer);

class Connector final {
 public:
  Connector(const HostPort &dest_addr, const TcpOptions &options,
            EventLoopSPtr loop);
  ~Connector();
  Connector(const Connector &t) = delete;
  Connector &operator=(const Connector &t) = delete;

  // call in loop thread
  int32_t TimerConnect(int64_t retry_times);
  int32_t Connect(int64_t retry_times);
  int32_t Connect();
  int32_t Close();

  void AssertInLoopThread();

  void set_new_conn_func(const ConnectorNewConnFunc &new_conn_func);
  const HostPort &dest_addr() { return dest_addr_; }

 private:
  // call in loop thread
  void Init();

 private:
  const HostPort dest_addr_;
  const TcpOptions options_;

  EventLoopWPtr loop_;
  UvTcpUPtr conn_;
  UvConnect connect_req_;
  TimerSPtr retry_timer_;
  ConnectorNewConnFunc new_conn_func_;

  friend void ConnectFinish(UvConnect *req, int32_t status);
};

inline void Connector::set_new_conn_func(
    const ConnectorNewConnFunc &new_conn_func) {
  new_conn_func_ = new_conn_func;
}

}  // namespace vraft

#endif