#ifndef VRAFT_EVENTLOOP_H_
#define VRAFT_EVENTLOOP_H_

#include <functional>
#include <memory>
#include <thread>

#include "async_queue.h"
#include "async_stop.h"
#include "timer.h"
#include "uv_wrapper.h"

namespace vraft {

class EventLoop;
using EventLoopPtr = std::shared_ptr<EventLoop>;

class EventLoop final {
 public:
  EventLoop(const std::string &name);
  ~EventLoop();
  EventLoop(const EventLoop &loop) = delete;
  EventLoop &operator=(const EventLoop &loop) = delete;

  // call in any thread
  void Stop();

  // call in loop thread
  int32_t Loop();
  bool IsAlive() const;
  bool IsInLoopThread() const;
  void AssertInLoopThread() const;
  TimerPtr MakeTimer(uint64_t timeout_ms, uint64_t repeat_ms,
                     const TimerFunctor &func, void *data);

  // call in any thread
  // 1. do not care the return value
  // 2. if you care, func will send the return value back
  void RunFunctor(const Functor func);
  void AddTimer(uint64_t timeout_ms, uint64_t repeat_ms,
                const TimerFunctor &func);
  void AddTimer(TimerPtr timer);
  void RemoveTimer(TimerId id);

  const std::string &name() const { return name_; }
  std::thread::id tid() const { return tid_; }
  const std::string &tid_str() const { return tid_str_; }
  UvLoop *UvLoopPtr() { return &uv_loop_; }

 private:
  void Init();
  void AddTimerInLoop(uint64_t timeout_ms, uint64_t repeat_ms,
                      const TimerFunctor &func);
  void AddTimerPtr(TimerPtr timer);  // overload function cannot bind
  void RemoveTimerLoop(TimerId id);

 private:
  const std::string name_;
  std::thread::id tid_;
  std::string tid_str_;

  UvLoop uv_loop_;
  TimerMap timers_;
  AsyncQueue functors_;
  AsyncStop async_stop_;
};

}  // namespace vraft

#endif
