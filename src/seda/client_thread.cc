#include "client_thread.h"

namespace vraft {

int32_t ClientThread::Start() {
  int32_t rv = 0;
  for (auto &item : clients_) {
    rv = item.second->Connect();
    assert(rv == 0);
  }

  stop_ =
      std::make_unique<CountDownLatch>(static_cast<int32_t>(clients_.size()));

  rv = loop_thread_->Start();
  assert(rv == 0);

  return 0;
}

void ClientThread::Stop() {
  for (auto &item : clients_) {
    item.second->Stop();
  }

  WaitServerClose();
  loop_thread_->Stop();
}

void ClientThread::Join() {
  assert(!detach_);
  loop_thread_->Join();
}

void ClientThread::AddClient(TcpClientSPtr client) {}

TcpClientSPtr ClientThread::GetClient(uint64_t dest_addr) {}

void ClientThread::ServerCloseCountDown() { stop_->CountDown(); }

void ClientThread::WaitServerClose() { stop_->Wait(); }

}  // namespace vraft
