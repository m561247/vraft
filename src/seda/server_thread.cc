#include "server_thread.h"

namespace vraft {

ServerThread::ServerThread(const ServerThreadParam &param)
    : name_(param.name),
      server_num_(param.server_num),
      detach_(param.detach),
      host_(param.host),
      start_port_(param.start_port) {
  loop_thread_ = std::make_shared<LoopThread>(name_, detach_);

  for (int32_t i = 0; i < server_num_; ++i) {
    char name_buf[128];
    snprintf(name_buf, sizeof(name_buf), "%s-%d", name_.c_str(), i);

    auto sptr = loop_thread_->loop();
    TcpServerSPtr tcp_server = std::make_shared<vraft::TcpServer>(
        sptr, name_buf, vraft::HostPort(host_, start_port_ + i), param.options);
    tcp_server->set_on_connection_cb(param.on_connection_cb);
    tcp_server->set_on_message_cb(param.on_message_cb);
    tcp_server->set_close_cb(
        std::bind(&ServerThread::ServerCloseCountDown, this));

    servers_.push_back(tcp_server);
  }

  stop_ =
      std::make_unique<CountDownLatch>(static_cast<int32_t>(servers_.size()));
}

int32_t ServerThread::Start() {
  int32_t rv = 0;
  for (auto &sptr : servers_) {
    rv = sptr->Start();
    assert(rv == 0);
  }

  rv = loop_thread_->Start();
  assert(rv == 0);

  return 0;
}

void ServerThread::Stop() {
  for (auto &sptr : servers_) {
    sptr->Stop();
  }

  WaitServerClose();
  loop_thread_->Stop();
}

void ServerThread::Join() {
  assert(!detach_);
  loop_thread_->Join();
}

EventLoopSPtr ServerThread::LoopPtr() { return loop_thread_->loop(); }

void ServerThread::ServerCloseCountDown() { stop_->CountDown(); }

void ServerThread::WaitServerClose() { stop_->Wait(); }

}  // namespace vraft
