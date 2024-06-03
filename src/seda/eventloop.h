#ifndef VRAFT_EVENTLOOP_H_
#define VRAFT_EVENTLOOP_H_

#include <functional>
#include <memory>
#include <thread>

#include "async_queue.h"
#include "async_stop.h"
#include "common.h"
#include "timer.h"
#include "uv_wrapper.h"

namespace vraft {

class EventLoop : public std::enable_shared_from_this<EventLoop> {
 public:
  EventLoop(const std::string &name);
  ~EventLoop();
  EventLoop(const EventLoop &loop) = delete;
  EventLoop &operator=(const EventLoop &loop) = delete;

  // call in any thread
  void Stop();

  // 1. do not care the return value
  // 2. if you care, func will send the return value back
  void RunFunctor(const Functor func);

  // call in loop thread
  // loop
  int32_t Init();
  int32_t Loop();
  bool Alive() const;
  bool IsInLoopThread() const;
  void AssertInLoopThread() const;

  // timer
  TimerSPtr MakeTimer(TimerParam &param);
  void AddTimer(TimerParam &param);
  void AddTimer(TimerSPtr timer);
  void RemoveTimer(TimerId id);

  // get/set
  UvLoop *UvLoopPtr();
  int32_t tid() const;
  const std::string &name() const;

  // uv_close callback
  void ResetAsyncQueue();
  void ResetAsyncStop();

 private:
  void Close();

 private:
  const std::string name_;
  int32_t tid_;
  UvLoop uv_loop_;

  TimerMap timers_;
  AsyncQueueSPtr functors_;
  AsyncStopSPtr stop_;

  friend void StopLoop(UvAsync *uv_async);
};

inline UvLoop *EventLoop::UvLoopPtr() { return &uv_loop_; }

inline int32_t EventLoop::tid() const { return tid_; }

inline const std::string &EventLoop::name() const { return name_; }

inline void EventLoop::ResetAsyncQueue() { functors_.reset(); }

inline void EventLoop::ResetAsyncStop() { stop_.reset(); }

}  // namespace vraft

#endif
