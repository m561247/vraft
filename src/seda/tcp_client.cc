#include "tcp_client.h"

namespace vraft {

int32_t TcpClient::TimerConnect(int64_t retry_times) {
  vraft_logger.FInfo("tcp-client:%s timer-connect, retry_times:%lu",
                     name_.c_str(), retry_times);
  return connector_.TimerConnect(retry_times);
}

int32_t TcpClient::Connect(int64_t retry_times) {
  vraft_logger.FInfo("tcp-client:%s connect, retry_times:%lu", name_.c_str(),
                     retry_times);
  return connector_.Connect(retry_times);
}

int32_t TcpClient::Connect() {
  vraft_logger.FInfo("tcp-client:%s connect", name_.c_str());
  return connector_.Connect();
}

int32_t TcpClient::Stop() {
  loop_->RunFunctor(std::bind(&TcpClient::StopInLoop, this));
  return 0;
}

int32_t TcpClient::StopInLoop() {
  loop_->AssertInLoopThread();
  vraft_logger.FInfo("tcp-client:%s stop", name_.c_str());
  return connector_.Stop();
}

int32_t TcpClient::Send(const char *buf, unsigned int size) {
  loop_->RunFunctor(std::bind(&TcpClient::SendInLoop, this, buf, size));
  return 0;
}

int32_t TcpClient::CopySend(const char *buf, unsigned int size) {
  loop_->RunFunctor(std::bind(&TcpClient::CopySendInLoop, this, buf, size));
  return 0;
}

int32_t TcpClient::BufSend(const char *buf, unsigned int size) {
  loop_->RunFunctor(std::bind(&TcpClient::BufSendInLoop, this, buf, size));
  return 0;
}

int32_t TcpClient::RemoveConnection(const TcpConnectionPtr &conn) {
  int32_t r = 0;
  if (conn->loop()->IsInLoopThread()) {
    r = RemoveConnectionInLoop(conn);
  } else {
    conn->loop()->RunFunctor(
        std::bind(&TcpClient::RemoveConnectionInLoop, this, conn));
  }
  return r;
}

int32_t TcpClient::SendInLoop(const char *buf, unsigned int size) {
  loop_->AssertInLoopThread();
  if (connection_) {
    return connection_->Send(buf, size);
  } else {
    return -1;
  }
}

int32_t TcpClient::CopySendInLoop(const char *buf, unsigned int size) {
  loop_->AssertInLoopThread();
  if (connection_) {
    return connection_->CopySend(buf, size);
  } else {
    return -1;
  }
}

int32_t TcpClient::BufSendInLoop(const char *buf, unsigned int size) {
  loop_->AssertInLoopThread();
  if (connection_) {
    return connection_->BufSend(buf, size);
  } else {
    return -1;
  }
}

int32_t TcpClient::RemoveConnectionInLoop(const TcpConnectionPtr &conn) {
  connection_.reset();
  return 0;
}

void TcpClient::NewConnection(UvTcpUPtr client) {
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

  connection_ =
      std::make_shared<TcpConnection>(loop_, conn_name, std::move(client));
  connection_->set_on_connection_cb(on_connection_cb_);
  connection_->set_write_complete_cb(write_complete_cb_);
  connection_->set_connection_close_cb(
      std::bind(&TcpClient::RemoveConnection, this, std::placeholders::_1));
  int32_t r = connection_->Start();
  assert(r == 0);

  if (on_connection_cb_) {
    on_connection_cb_(connection_);
  }
}

void TcpClient::Init() {
  connector_.set_new_conn_func(
      std::bind(&TcpClient::NewConnection, this, std::placeholders::_1));
}

} // namespace vraft