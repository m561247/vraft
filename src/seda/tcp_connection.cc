#include "tcp_connection.h"

#include <cassert>
#include <iostream>

#include "allocator.h"
#include "eventloop.h"
#include "hostport.h"
#include "vraft_logger.h"

namespace vraft {

TcpConnection::TcpConnection(EventLoop *loop, const std::string &name,
                             UvTcpUPtr conn)
    : name_(name), loop_(loop), allocator_(), conn_(std::move(conn)) {
  vraft_logger.FInfo("connection:%s construct, handle:%p", name_.c_str(),
                     conn_.get());
  conn_->data = this;

  sockaddr_in local_addr, peer_addr;
  int namelen = sizeof(sockaddr_in);
  int rv = 0;

  rv =
      UvTcpGetSockName(conn_.get(), (struct sockaddr *)(&local_addr), &namelen);
  assert(rv == 0);
  local_addr_ = SockaddrInToHostPort(&local_addr);

  rv = UvTcpGetPeerName(conn_.get(), (struct sockaddr *)(&peer_addr), &namelen);
  assert(rv == 0);
  peer_addr_ = SockaddrInToHostPort(&peer_addr);
}

TcpConnection::~TcpConnection() {
  vraft_logger.FInfo("connection:%s destruct, handle:%p", name_.c_str(),
                     conn_.get());
  Stop();
}

void HandleClientClose(UvHandle *client) {
  TcpConnection *conn = static_cast<TcpConnection *>(client->data);
  conn->loop()->AssertInLoopThread();
  assert(conn->connection_close_cb_);
  TcpConnectionPtr cptr = conn->shared_from_this();
  conn->connection_close_cb_(cptr);
}

void TcpConnectionHandleRead(UvStream *client, ssize_t nread,
                             const UvBuf *buf) {
  TcpConnection *conn = static_cast<TcpConnection *>(client->data);
  conn->loop()->AssertInLoopThread();

  if (nread > 0) {
    uint32_t u32 = Crc32(buf->base, nread);
    vraft_logger.FTrace("recv data, handle:%p, nread:%d, check:%X", client,
                        nread, u32);
    vraft_logger.FDebug("recv data:%s", StrToHexStr(buf->base, nread).c_str());
    conn->OnMessage(buf->base, nread);

  } else {
    vraft_logger.FError("read error, handle:%p, nread:%d, err:%s", client,
                        nread, UvStrError(nread));
    UvClose(reinterpret_cast<UvHandle *>(client), HandleClientClose);
  }
}

void TcpConnectionAllocBuffer(UvHandle *client, size_t suggested_size,
                              UvBuf *buf) {
  TcpConnection *conn = static_cast<TcpConnection *>(client->data);
  conn->loop()->AssertInLoopThread();

  buf->base = (char *)conn->allocator().Malloc(suggested_size);
  buf->len = suggested_size;
}

void TcpConnectionAllocBuffer2(UvHandle *client, size_t suggested_size,
                               UvBuf *buf) {
  TcpConnection *conn = static_cast<TcpConnection *>(client->data);
  conn->loop()->AssertInLoopThread();

  buf->base = const_cast<char *>(conn->input_buf_.BeginWrite());
  buf->len = conn->input_buf_.WritableBytes();
}

void WriteComplete(UvWrite *req, int status) {
  TcpConnection *conn = static_cast<TcpConnection *>(req->handle->data);
  conn->loop()->AssertInLoopThread();

  assert(status == 0);
  WriteReq *wr = reinterpret_cast<WriteReq *>(req);

  // only free wr
  // user need to free buf
  conn->allocator().Free(wr);

  if (conn->write_complete_cb_) {
    conn->write_complete_cb_(conn->shared_from_this());
  }
}

void BufWriteComplete(UvWrite *req, int status) {
  TcpConnection *conn = static_cast<TcpConnection *>(req->handle->data);
  conn->loop()->AssertInLoopThread();

  assert(status == 0);
  WriteReq *wr = reinterpret_cast<WriteReq *>(req);
  conn->allocator().Free(wr->buf.base);
  conn->allocator().Free(wr);

  if (conn->write_complete_cb_) {
    conn->write_complete_cb_(conn->shared_from_this());
  }
}

#if 0
int32_t TcpConnection::Start() {
  loop_->AssertInLoopThread();
  return UvReadStart(reinterpret_cast<UvStream *>(conn_.get()),
                     TcpConnectionAllocBuffer, TcpConnectionHandleRead);
}
#endif

int32_t TcpConnection::Start() {
  loop_->AssertInLoopThread();
  return UvReadStart(reinterpret_cast<UvStream *>(conn_.get()),
                     TcpConnectionAllocBuffer2, TcpConnectionHandleRead);
}

int32_t TcpConnection::Stop() {
  loop_->AssertInLoopThread();
  if (!UvIsClosing(reinterpret_cast<UvHandle *>(conn_.get()))) {
    UvClose(reinterpret_cast<UvHandle *>(conn_.get()), HandleClientClose);
  } else {
    vraft_logger.FInfo("connection:%s close, already closing, handle:%p",
                       name_.c_str(), conn_.get());
  }
  return 0;
}

bool TcpConnection::Connected() const {
  return UvIsActive(reinterpret_cast<UvHandle *>(conn_.get())) &&
         UvIsReadable(reinterpret_cast<UvStream *>(conn_.get())) &&
         UvIsWritable(reinterpret_cast<UvStream *>(conn_.get()));
}

void TcpConnection::OnMessage(const char *buf, ssize_t size) {
  loop_->AssertInLoopThread();

  input_buf_.Append(buf, size);
  if (on_message_cb_) {
    on_message_cb_(shared_from_this(), &input_buf_);
  }
}

void TcpConnection::OnMessage(Buffer &buf) {
  loop_->AssertInLoopThread();

  if (on_message_cb_) {
    on_message_cb_(shared_from_this(), &input_buf_);
  }
}

int32_t TcpConnection::Send(const char *buf, ssize_t size) {
  loop_->AssertInLoopThread();

  if (!Connected()) {
    vraft_logger.FError("send error, connection:%s not connected",
                        name_.c_str());
  }

  WriteReq *write_req = (WriteReq *)allocator_.Malloc(sizeof(WriteReq));
  write_req->buf =
      UvBufInit(const_cast<char *>(buf), static_cast<unsigned int>(size));
  UvWrite2(reinterpret_cast<UvWrite *>(write_req),
           reinterpret_cast<UvStream *>(conn_.get()), &write_req->buf, 1,
           WriteComplete);
  return 0;
}

int32_t TcpConnection::CopySend(const char *buf, ssize_t size) {
  loop_->AssertInLoopThread();

  if (!Connected()) {
    vraft_logger.FError("send error, connection:%s not connected",
                        name_.c_str());
  }

  char *send_buf = (char *)allocator_.Malloc(size);
  memcpy(send_buf, buf, size);

  WriteReq *write_req = (WriteReq *)allocator_.Malloc(sizeof(WriteReq));
  write_req->buf =
      UvBufInit(const_cast<char *>(send_buf), static_cast<unsigned int>(size));
  UvWrite2(reinterpret_cast<UvWrite *>(write_req),
           reinterpret_cast<UvStream *>(conn_.get()), &write_req->buf, 1,
           BufWriteComplete);
  return 0;
}

int32_t TcpConnection::BufSend(const char *buf, ssize_t size) {
  loop_->AssertInLoopThread();

  if (!Connected()) {
    vraft_logger.FError("send error, connection:%s not connected",
                        name_.c_str());
  }

  char *send_buf = (char *)allocator_.Malloc(size);
  memcpy(send_buf, buf, size);

  WriteReq *write_req = (WriteReq *)allocator_.Malloc(sizeof(WriteReq));
  write_req->buf =
      UvBufInit(const_cast<char *>(send_buf), static_cast<unsigned int>(size));
  UvWrite2(reinterpret_cast<UvWrite *>(write_req),
           reinterpret_cast<UvStream *>(conn_.get()), &write_req->buf, 1,
           BufWriteComplete);
  return 0;
}

}  // namespace vraft
