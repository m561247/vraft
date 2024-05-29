#ifndef VRAFT_TCP_CONNECTION_H_
#define VRAFT_TCP_CONNECTION_H_

#include <functional>
#include <map>
#include <memory>

#include "allocator.h"
#include "buffer.h"
#include "hostport.h"
#include "uv_wrapper.h"

namespace vraft {

class EventLoop;
using UvTcpUPtr = std::unique_ptr<UvTcp>;

class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TcpConnectionUPtr = std::unique_ptr<TcpConnection>;
using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

using OnConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
using OnMessageCallback = std::function<void(TcpConnectionPtr, Buffer *)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
using ConnectionCloseCallback =
    std::function<int32_t(const TcpConnectionPtr &)>;

void TcpConnectionHandleRead(UvStream *client, ssize_t nread, const UvBuf *buf);
void TcpConnectionAllocBuffer(UvHandle *handle, size_t suggested_size,
                              UvBuf *buf);
void TcpConnectionAllocBuffer2(UvHandle *handle, size_t suggested_size,
                               UvBuf *buf);
void WriteComplete(UvWrite *req, int status);
void BufWriteComplete(UvWrite *req, int status);
void HandleClientClose(UvHandle *client);

struct WriteReq {
  UvWrite req;
  UvBuf buf;
};

class TcpConnection final : public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop *loop, const std::string &name, UvTcpUPtr conn);
  ~TcpConnection();
  TcpConnection(const TcpConnection &t) = delete;
  TcpConnection &operator=(const TcpConnection &t) = delete;

  // call in loop thread
  int32_t Start();
  int32_t Close();
  bool Connected() const;

  // user need to free buf in write_complete_cb
  int32_t Send(const char *buf, ssize_t size);

  // copy data into connection buf, user need not care
  int32_t CopySend(const char *buf, ssize_t size);

  // buf is allocated by the allocator of connection
  int32_t BufSend(const char *buf, ssize_t size);

  void OnMessage(const char *buf, ssize_t size);
  void OnMessage(Buffer &buf);

  void set_on_connection_cb(const OnConnectionCallback &cb);
  void set_on_message_cb(const OnMessageCallback &cb);
  void set_write_complete_cb(const WriteCompleteCallback &cb);
  void set_connection_close_cb(const ConnectionCloseCallback &cb);

  EventLoop *loop();
  Allocator &allocator();
  const std::string &name() const;
  HostPort local_addr() const;
  HostPort peer_addr() const;

 private:
  const std::string name_;
  HostPort local_addr_;
  HostPort peer_addr_;

  EventLoop *loop_;
  Allocator allocator_;
  UvTcpUPtr conn_;
  Buffer input_buf_;

  OnConnectionCallback on_connection_cb_;
  OnMessageCallback on_message_cb_;
  WriteCompleteCallback write_complete_cb_;
  ConnectionCloseCallback connection_close_cb_;

  friend void HandleClientClose(UvHandle *client);
  friend void WriteComplete(UvWrite *req, int status);
  friend void BufWriteComplete(UvWrite *req, int status);
  friend void TcpConnectionAllocBuffer2(UvHandle *handle, size_t suggested_size,
                                        UvBuf *buf);
};

inline void TcpConnection::set_on_connection_cb(
    const OnConnectionCallback &cb) {
  on_connection_cb_ = cb;
}

inline void TcpConnection::set_on_message_cb(const OnMessageCallback &cb) {
  on_message_cb_ = cb;
}

inline void TcpConnection::set_write_complete_cb(
    const WriteCompleteCallback &cb) {
  write_complete_cb_ = cb;
}

inline void TcpConnection::set_connection_close_cb(
    const ConnectionCloseCallback &cb) {
  connection_close_cb_ = cb;
}

inline const std::string &TcpConnection::name() const { return name_; }

inline HostPort TcpConnection::local_addr() const { return local_addr_; }

inline HostPort TcpConnection::peer_addr() const { return peer_addr_; }

inline EventLoop *TcpConnection::loop() { return loop_; }

inline Allocator &TcpConnection::allocator() { return allocator_; }

}  // namespace vraft

#endif
