#include "server_thread.h"

namespace vraft {

ServerThread::ServerThread(const std::string &name, bool detach)
    : name_(name), detach_(detach) {
  loop_thread_ = std::make_shared<LoopThread>(name_, detach);
}

int32_t ServerThread::Start() {
  int32_t rv = 0;
  for (auto &wptr : servers_) {
    auto sptr = wptr.lock();
    if (sptr) {
      rv = sptr->Start();
      assert(rv == 0);
    }
  }

  stop_ =
      std::make_unique<CountDownLatch>(static_cast<int32_t>(servers_.size()));

  rv = loop_thread_->Start();
  assert(rv == 0);

  return 0;
}

void ServerThread::Stop() {
  for (auto &wptr : servers_) {
    auto sptr = wptr.lock();
    if (sptr) {
      sptr->Stop();
    }
  }

  WaitServerClose();
  loop_thread_->Stop();
}

void ServerThread::Join() {
  assert(!detach_);
  loop_thread_->Join();
}

void ServerThread::AddServer(TcpServerSPtr sptr) { servers_.push_back(sptr); }

EventLoopSPtr ServerThread::LoopPtr() { return loop_thread_->loop(); }

void ServerThread::ServerCloseCountDown() { stop_->CountDown(); }

void ServerThread::WaitServerClose() { stop_->Wait(); }

}  // namespace vraft
