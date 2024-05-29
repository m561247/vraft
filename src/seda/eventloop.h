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

  // call in any thread
  // 1. do not care the return value
  // 2. if you care, func will send the return value back
  void RunFunctor(const Functor func);

  // call in loop thread
  int32_t Loop();
  bool IsAlive() const;
  bool IsInLoopThread() const;
  void AssertInLoopThread() const;
  void StopInLoop();

  // call in loop thread
  TimerPtr MakeTimer(TimerParam &param);
  void AddTimer(TimerParam &param);
  void AddTimer(TimerPtr timer);
  void RemoveTimer(TimerId id);

  // call in loop thread
  UvLoop *UvLoopPtr();
  std::thread::id tid() const;
  const std::string &tid_str() const;
  const std::string &name() const;

 private:
  void Init();
  void CloseResource();

 private:
  const std::string name_;
  std::thread::id tid_;
  std::string tid_str_;

  UvLoop uv_loop_;
  TimerMap timers_;
  AsyncQueue functors_;
  AsyncStop async_stop_;
};

inline UvLoop *EventLoop::UvLoopPtr() { return &uv_loop_; }

inline std::thread::id EventLoop::tid() const { return tid_; }

inline const std::string &EventLoop::tid_str() const { return tid_str_; }

inline const std::string &EventLoop::name() const { return name_; }

}  // namespace vraft

#endif
