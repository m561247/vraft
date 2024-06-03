#include "eventloop.h"

#include <sys/types.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "clock.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

EventLoop::EventLoop(const std::string &name) : name_(name), tid_(0) {
  vraft_logger.FInfo("loop:%p construct, name:%s", &uv_loop_, name_.c_str());
  int32_t rv = UvLoopInit(&uv_loop_);
  assert(rv == 0);
}

EventLoop::~EventLoop() {
  int32_t rv = UvLoopClose(&uv_loop_);
  vraft_logger.FInfo("loop:%p destruct, name:%s, rv:%d, %s", &uv_loop_,
                     name_.c_str(), rv, UvStrError(rv));
}

void EventLoop::Stop() {
  vraft_logger.FInfo("loop:%p async stop, name:%s, tid:%d", &uv_loop_,
                     name_.c_str(), tid_);
  if (IsInLoopThread()) {
    Close();
  } else {
    stop_->Notify();
  }
}

void EventLoop::RunFunctor(const Functor func) {
  if (IsInLoopThread()) {
    func();
  } else {
    functors_->Push(std::move(func));
  }
}

int32_t EventLoop::Init() {
  auto sptr = shared_from_this();
  functors_ = std::make_shared<AsyncQueue>(sptr);
  stop_ = std::make_shared<AsyncStop>(sptr);
  return 0;
}

int32_t EventLoop::Loop() {
  tid_ = gettid();
  vraft_logger.FInfo("loop:%p start, name:%s, tid:%d", &uv_loop_, name_.c_str(),
                     tid_);
  return UvRun(&uv_loop_, UV_RUN_DEFAULT);
}

bool EventLoop::Alive() const {
  AssertInLoopThread();
  return UvLoopAlive(&uv_loop_);
}

bool EventLoop::IsInLoopThread() const {
  if (tid_ != 0) {
    return (gettid() == tid_);
  } else {
    return true;
  }
}

void EventLoop::AssertInLoopThread() const {
  if (!IsInLoopThread()) {
    vraft_logger.FFatal(
        "AssertInLoopThread, loop:%p, name:%s, loop-tid:%d, current-tid:%d",
        &uv_loop_, name_.c_str(), tid_, gettid());
  }
}

TimerSPtr EventLoop::MakeTimer(TimerParam &param) {
  AssertInLoopThread();
  auto sptr = shared_from_this();
  TimerSPtr ptr = CreateTimer(param, sptr);
  return ptr;
}

void EventLoop::AddTimer(TimerParam &param) {
  AssertInLoopThread();
  TimerSPtr ptr = MakeTimer(param);
  ptr->Start();
  vraft_logger.FInfo("loop:%p %s add timer:%p %ld %s", &uv_loop_, name_.c_str(),
                     ptr->UvTimerPtr(), ptr->id(), ptr->name().c_str());
  timers_[ptr->id()] = std::move(ptr);
}

void EventLoop::AddTimer(TimerSPtr timer) {
  AssertInLoopThread();
  timer->Start();
  vraft_logger.FInfo("loop:%p %s add timer:%p %ld %s", &uv_loop_, name_.c_str(),
                     timer->UvTimerPtr(), timer->id(), timer->name().c_str());
  timers_[timer->id()] = std::move(timer);
}

void EventLoop::RemoveTimer(TimerId id) {
  AssertInLoopThread();
  vraft_logger.FInfo("loop:%p %s remove timer:%ld", &uv_loop_, name_.c_str(),
                     id);
  timers_.erase(id);
}

void EventLoop::Close() {
  AssertInLoopThread();
  TimerMap timers2 = timers_;
  for (auto &t : timers2) {
    t.second->Stop();
    t.second->Close();
  }
  stop_->Close();
  functors_->Close();
}

}  // namespace vraft
