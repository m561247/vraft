#ifndef VRAFT_TIMER_H_
#define VRAFT_TIMER_H_

#include <atomic>
#include <map>
#include <memory>

#include "common.h"
#include "uv_wrapper.h"

namespace vraft {

class Timer;
using TimerPtr = std::shared_ptr<Timer>;
using TimerFunctor = std::function<void(Timer *)>;

using TimerId = int64_t;
using TimerMap = std::map<TimerId, TimerPtr>;

struct TimerParam {
  uint64_t timeout_ms;
  uint64_t repeat_ms;
  TimerFunctor cb;
  void *data;
};
using MakeTimerFunc = std::function<TimerPtr(TimerParam &param)>;

class EventLoop;
TimerPtr CreateTimer(TimerParam &param, EventLoop *loop);
void HandleUvTimer(UvTimer *uv_timer);

class Timer final {
 public:
  Timer(TimerParam &param, EventLoop *loop);
  ~Timer();
  Timer(const Timer &t) = delete;
  Timer &operator=(const Timer &t) = delete;

  // call in loop thread
  int32_t Start();
  int32_t Stop();
  int32_t Close();
  int32_t Again();
  int32_t Again(uint64_t timeout_ms, uint64_t repeat_ms);
  bool IsStart();

  EventLoop *loop() const { return loop_; }
  TimerId id() const { return id_; }

  int64_t RepeatSubOne() { return --repeat_counter_; }
  int64_t repeat_counter() const { return repeat_counter_; }
  int64_t repeat_times() const { return repeat_times_; }

  void set_repeat_times(int64_t repeat_times) {
    repeat_times_ = repeat_times;
    repeat_counter_ = repeat_times;
  }

  uint64_t dest_addr() { return dest_addr_; }
  void set_dest_addr(uint64_t dest_addr) { dest_addr_ = dest_addr; }
  void *data() { return data_; }
  void set_data(void *data) { data_ = data; }

 private:
  void Init();

 private:
  TimerId id_;
  uint64_t timeout_ms_;
  uint64_t repeat_ms_;
  int64_t repeat_times_;
  int64_t repeat_counter_;
  uint64_t dest_addr_;
  const TimerFunctor cb_;
  void *data_;

  EventLoop *loop_;
  UvTimer uv_timer_;

  // maybe serval threads create Timer object at the same time
  static std::atomic<int64_t> seq_;

  friend void HandleUvTimer(UvTimer *uv_timer);
};

}  // namespace vraft

#endif
