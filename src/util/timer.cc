#include "timer.h"

#include <cassert>

#include "eventloop.h"
#include "vraft_logger.h"

namespace vraft {

TimerPtr CreateTimer(uint64_t timeout, uint64_t repeat, EventLoop *loop,
                     const TimerFunctor &cb) {
  TimerPtr ptr = std::make_shared<Timer>(timeout, repeat, loop, cb);
  return std::move(ptr);
}

void HandleUvTimer(UvTimer *uv_timer) {
  Timer *timer = reinterpret_cast<Timer *>(uv_timer->data);
  assert(timer != nullptr);
  timer->loop()->AssertInLoopThread();

  if (timer->cb_) {
    timer->cb_(timer);
  }
}

Timer::Timer(uint64_t timeout_ms, uint64_t repeat_ms, EventLoop *loop,
             const TimerFunctor &cb)
    : data(nullptr),
      id_(seq_.fetch_add(1)),
      timeout_ms_(timeout_ms),
      repeat_ms_(repeat_ms),
      cb_(cb),
      repeat_times_(0),
      loop_(loop) {
  vraft_logger.FInfo(
      "timer:%ld construct, timeout_ms:%lu, repeat_ms:%lu, loop:%s, handle:%p",
      id_, timeout_ms_, repeat_ms_, loop_->name().c_str(), &uv_timer_);
  Init();
}

Timer::~Timer() {
  vraft_logger.FInfo(
      "timer:%ld destruct, timeout_ms:%lu, repeat_ms:%lu, loop:%s, handle:%p",
      id_, timeout_ms_, repeat_ms_, loop_->name().c_str(), &uv_timer_);
  Stop();
}

void Timer::Init() {
  UvTimerInit(loop_->UvLoopPtr(), &uv_timer_);
  uv_timer_.data = this;
}

int32_t Timer::Start() {
  // maybe loop not start
  loop_->AssertInLoopThread();
  int32_t r = UvimerStart(&uv_timer_, HandleUvTimer, timeout_ms_, repeat_ms_);
  assert(r == 0);
  return r;
}

int32_t Timer::Stop() {
  // maybe loop not start
  loop_->AssertInLoopThread();
  int32_t r = 0;
  if (!UvIsClosing(reinterpret_cast<UvHandle *>(&uv_timer_))) {
    r = UvTimerStop(&uv_timer_);  // equal to uv_close
    assert(r == 0);
  } else {
    vraft_logger.FInfo(
        "timer:%ld stop, already stopping, timeout_ms:%lu, "
        "repeat_ms:%lu, loop:%s, handle:%p",
        id_, timeout_ms_, repeat_ms_, loop_->name().c_str(), &uv_timer_);
  }
  loop_->RemoveTimer(id_);
  return r;
}

int32_t Timer::Again() {
  loop_->AssertInLoopThread();
  int32_t r = UvTimerAgain(&uv_timer_);
  assert(r == 0);
  return r;
}

int32_t Timer::Again(uint64_t timeout_ms, uint64_t repeat_ms) {
  loop_->AssertInLoopThread();
  timeout_ms_ = timeout_ms;
  repeat_ms_ = repeat_ms;

  int32_t r = UvTimerStop(&uv_timer_);
  assert(r == 0);

  r = UvimerStart(&uv_timer_, HandleUvTimer, timeout_ms_, repeat_ms_);
  assert(r == 0);
  return r;
}

bool Timer::IsStart() {
  loop_->AssertInLoopThread();
  return UvIsActive(reinterpret_cast<UvHandle *>(&uv_timer_));
}

std::atomic<int64_t> Timer::seq_(0);

}  // namespace vraft
