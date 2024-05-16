#include "async_stop.h"

#include "eventloop.h"
#include "vraft_logger.h"

namespace vraft {

void StopLoop(UvAsync *uv_async) {
  AsyncStop *async_stop = static_cast<AsyncStop *>(uv_async->data);
  async_stop->AssertInLoopThread();
  vraft_logger.FInfo("loop:%s stop, tid:%s", async_stop->loop_->name().c_str(),
                     async_stop->loop_->tid_str().c_str());
  UvStop(async_stop->loop_->UvLoopPtr());
}

void AsyncStop::Init(EventLoop *loop) {
  // loop->AssertInLoopThread();
  // loop not start

  loop_ = loop;
  UvAsyncInit(loop_->UvLoopPtr(), &uv_async_, StopLoop);
  uv_async_.data = this;
}

void AsyncStop::AssertInLoopThread() { loop_->AssertInLoopThread(); }

void AsyncStop::Notify() { UvAsyncSend(&uv_async_); }

}  // namespace vraft
