#include "timer.h"

#include <cassert>

#include "eventloop.h"
#include "vraft_logger.h"

namespace vraft {

TimerPtr CreateTimer(TimerParam &param, EventLoop *loop) {
  TimerPtr ptr = std::make_shared<Timer>(param, loop);
  return ptr;
}

void HandleUvTimer(UvTimer *uv_timer) {
  Timer *timer = reinterpret_cast<Timer *>(uv_timer->data);
  assert(timer != nullptr);
  timer->loop()->AssertInLoopThread();

  if (timer->cb_) {
    timer->cb_(timer);
  }
}

Timer::Timer(TimerParam &param, EventLoop *loop)
    : id_(seq_.fetch_add(1)),
      timeout_ms_(param.timeout_ms),
      repeat_ms_(param.repeat_ms),
      repeat_times_(param.repeat_times),
      repeat_counter_(param.repeat_times),
      cb_(param.cb),
      data_(param.data),
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
}

void Timer::Init() {
  UvTimerInit(loop_->UvLoopPtr(), &uv_timer_);
  uv_timer_.data = this;
}

int32_t Timer::Start() {
  loop_->AssertInLoopThread();
  int32_t rv = UvimerStart(&uv_timer_, HandleUvTimer, timeout_ms_, repeat_ms_);
  assert(rv == 0);
  return rv;
}

int32_t Timer::Stop() {
  loop_->AssertInLoopThread();
  int32_t rv = 0;
  if (!UvIsClosing(reinterpret_cast<UvHandle *>(&uv_timer_))) {
    rv = UvTimerStop(&uv_timer_);  // equal to uv_close
    assert(rv == 0);
  } else {
    vraft_logger.FInfo(
        "timer:%ld stop, already stopping, timeout_ms:%lu, "
        "repeat_ms:%lu, loop:%s, handle:%p",
        id_, timeout_ms_, repeat_ms_, loop_->name().c_str(), &uv_timer_);
  }
  return rv;
}

void TimerCloseCb(UvHandle *handle) {
  Timer *ptr = reinterpret_cast<Timer *>(handle->data);
  ptr->loop()->RemoveTimer(ptr->id());
}

int32_t Timer::Close() {
  loop_->AssertInLoopThread();
  UvClose(reinterpret_cast<uv_handle_t *>(&uv_timer_), TimerCloseCb);
  return 0;
}

int32_t Timer::Again() {
  loop_->AssertInLoopThread();
  int32_t rv = UvTimerAgain(&uv_timer_);
  assert(rv == 0);
  return rv;
}

int32_t Timer::Again(uint64_t timeout_ms, uint64_t repeat_ms) {
  loop_->AssertInLoopThread();
  timeout_ms_ = timeout_ms;
  repeat_ms_ = repeat_ms;

  int32_t rv = UvTimerStop(&uv_timer_);
  assert(rv == 0);

  rv = UvimerStart(&uv_timer_, HandleUvTimer, timeout_ms_, repeat_ms_);
  assert(rv == 0);
  return rv;
}

bool Timer::IsStart() {
  loop_->AssertInLoopThread();
  return UvIsActive(reinterpret_cast<UvHandle *>(&uv_timer_));
}

std::atomic<int64_t> Timer::seq_(0);

}  // namespace vraft
