#ifndef VRAFT_TCP_CLIENT_H_
#define VRAFT_TCP_CLIENT_H_

#include <memory>
#include <string>

#include "common.h"
#include "connector.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "tcp_options.h"

namespace vraft {

class TcpClient;
using TcpClientPtr = std::shared_ptr<TcpClient>;

class TcpClient final {
 public:
  TcpClient(const std::string name, EventLoopSPtr loop,
            const HostPort &dest_addr, const TcpOptions &options);
  ~TcpClient();
  TcpClient(const TcpClient &t) = delete;
  TcpClient &operator=(const TcpClient &t) = delete;

  // my thread is loop thread
  int32_t TimerConnect(int64_t retry_times);
  int32_t Connect(int64_t retry_times);
  int32_t Connect();

  // call in any thread
  bool Connected() const;
  int32_t Stop();
  int32_t Send(const char *buf, unsigned int size);
  int32_t CopySend(const char *buf, unsigned int size);
  int32_t BufSend(const char *buf, unsigned int size);
  void RemoveConnection(const TcpConnectionSPtr &conn);

  // call in loop thread
  void set_on_connection_cb(const OnConnectionCallback &cb);
  void set_write_complete_cb(const WriteCompleteCallback &cb);
  void set_connection_close_cb(const ConnectionCloseCallback &cb);
  const std::string &name() const;
  std::string ToString() const;

  void AssertInLoopThread();

 private:
  void Init();
  void NewConnection(UvTcpUPtr client);

  int32_t StopInLoop();
  int32_t SendInLoop(const char *buf, unsigned int size);
  int32_t CopySendInLoop(const char *buf, unsigned int size);
  int32_t BufSendInLoop(const char *buf, unsigned int size);
  int32_t RemoveConnectionInLoop(const TcpConnectionSPtr &conn);

 private:
  const std::string name_;
  EventLoopWPtr loop_;
  Connector connector_;
  TcpConnectionSPtr connection_;

  OnConnectionCallback on_connection_cb_;
  WriteCompleteCallback write_complete_cb_;
  ConnectionCloseCallback connection_close_cb_;
};

inline TcpClient::TcpClient(const std::string name, EventLoopSPtr loop,
                            const HostPort &dest_addr,
                            const TcpOptions &options)
    : name_(name), loop_(loop), connector_(loop, dest_addr, options) {
  vraft_logger.FInfo("tcp-client:%s construct", name_.c_str());
  Init();
}

inline TcpClient::~TcpClient() {
  vraft_logger.FInfo("tcp-client:%s destruct", name_.c_str());
}

inline bool TcpClient::Connected() const {
  if (connection_) {
    return connection_->Connected();
  } else {
    return false;
  }
}

inline void TcpClient::set_on_connection_cb(const OnConnectionCallback &cb) {
  on_connection_cb_ = cb;
}

inline void TcpClient::set_write_complete_cb(const WriteCompleteCallback &cb) {
  write_complete_cb_ = cb;
}

inline void TcpClient::set_connection_close_cb(
    const ConnectionCloseCallback &cb) {
  connection_close_cb_ = cb;
}

inline const std::string &TcpClient::name() const { return name_; }

inline std::string TcpClient::ToString() const {
  if (connection_) {
    return connection_->ToString();
  } else {
    return "local:-peer:";
  }
}

}  // namespace vraft

#endif