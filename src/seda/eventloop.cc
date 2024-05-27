#include "eventloop.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "clock.h"
#include "util.h"
#include "vraft_logger.h"

namespace vraft {

EventLoop::EventLoop(const std::string &name) : name_(name) {
  vraft_logger.FInfo("loop:%s construct, handle:%p", name_.c_str(), &uv_loop_);
  Init();
}

EventLoop::~EventLoop() {
  vraft_logger.FInfo("loop:%s destruct, handle:%p", name_.c_str(), &uv_loop_);
  UvLoopClose(&uv_loop_);
}

void EventLoop::Init() {
  UvLoopInit(&uv_loop_);
  functors_.Init(this);
  async_stop_.Init(this);
}

int32_t EventLoop::Loop() {
  tid_ = std::this_thread::get_id();
  tid_str_ = TidToStr(tid_);
  vraft_logger.FInfo("loop:%s start, tid:%s, handle:%p", name_.c_str(),
                     tid_str_.c_str(), &uv_loop_);
  return UvRun(&uv_loop_, UV_RUN_DEFAULT);
}

void EventLoop::Stop() {
  vraft_logger.FInfo("loop:%s async stop, tid:%s, handle:%p", name_.c_str(),
                     tid_str_.c_str(), &uv_loop_);
  async_stop_.Notify();
}

bool EventLoop::IsAlive() const { return UvLoopAlive(&uv_loop_); }

bool EventLoop::IsInLoopThread() const {
  if (TidValid(tid_)) {
    return (std::this_thread::get_id() == tid_);
  } else {
    return true;
  }
}

void EventLoop::AssertInLoopThread() const {
  if (!IsInLoopThread()) {
    vraft_logger.FFatal(
        "AssertInLoopThread, loop:%s, handle:%p, loop-tid:%s, current-tid:%s",
        name_.c_str(), &uv_loop_, tid_str_.c_str(), CurrentTidStr().c_str());
  }
}

void EventLoop::RunFunctor(const Functor func) {
  if (IsInLoopThread()) {
    func();
  } else {
    functors_.Push(std::move(func));
  }
}

TimerPtr EventLoop::MakeTimer(uint64_t timeout_ms, uint64_t repeat_ms,
                              const TimerFunctor &func, void *data) {
  TimerPtr ptr = CreateTimer(timeout_ms, repeat_ms, this, func);
  ptr->set_data(data);
  return ptr;
}

void EventLoop::AddTimer(uint64_t timeout_ms, uint64_t repeat_ms,
                         const TimerFunctor &func) {
  if (IsInLoopThread()) {
    AddTimerInLoop(timeout_ms, repeat_ms, func);
  } else {
    Functor run_in_loop = std::bind(&EventLoop::AddTimerInLoop, this,
                                    timeout_ms, repeat_ms, func);
    RunFunctor(run_in_loop);
  }
}

void EventLoop::AddTimer(TimerPtr timer) {
  if (IsInLoopThread()) {
    AddTimerPtr(timer);
  } else {
    Functor run_in_loop = std::bind(&EventLoop::AddTimerPtr, this, timer);
    RunFunctor(run_in_loop);
  }
}

void EventLoop::RemoveTimer(TimerId id) {
  if (IsInLoopThread()) {
    RemoveTimerLoop(id);
  } else {
    Functor run_in_loop = std::bind(&EventLoop::RemoveTimerLoop, this, id);
    RunFunctor(run_in_loop);
  }
}

void EventLoop::AddTimerInLoop(uint64_t timeout_ms, uint64_t repeat_ms,
                               const TimerFunctor &func) {
  AssertInLoopThread();
  TimerPtr ptr = MakeTimer(timeout_ms, repeat_ms, func, nullptr);
  ptr->Start();
  vraft_logger.FInfo("loop:%s add timer:%ld", name_.c_str(), ptr->id());
  timers_[ptr->id()] = std::move(ptr);
}

void EventLoop::AddTimerPtr(TimerPtr timer) {
  AssertInLoopThread();
  timer->Start();
  vraft_logger.FInfo("loop:%s add timer:%ld", name_.c_str(), timer->id());
  timers_[timer->id()] = std::move(timer);
}

void EventLoop::RemoveTimerLoop(TimerId id) {
  AssertInLoopThread();
  vraft_logger.FInfo("loop:%s remove timer:%ld", name_.c_str(), id);
  timers_.erase(id);
}

}  // namespace vraft
