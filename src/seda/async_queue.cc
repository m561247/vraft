#include "async_queue.h"
#include "eventloop.h"

namespace vraft {

void AsyncQueueCb(UvAsync *uv_async) {
  AsyncQueue *async_queue = static_cast<AsyncQueue *>(uv_async->data);
  async_queue->DoFunctor();
}

void AsyncQueue::Init(EventLoop *loop) {
  // loop->AssertInLoopThread();
  // loop not start

  loop_ = loop;
  UvAsyncInit(loop_->UvLoopPtr(), &uv_async_, AsyncQueueCb);
  uv_async_.data = this;
}

void AsyncQueue::Push(const Functor func) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors_.push(func);
  }
  UvAsyncSend(&uv_async_);
}

void AsyncQueue::DoFunctor() {
  loop_->AssertInLoopThread();

  std::queue<Functor> todo_queue;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    todo_queue.swap(functors_);
  }
  while (!todo_queue.empty()) {
    auto func = todo_queue.front();
    func();
    todo_queue.pop();
  }
}

void AsyncQueue::AssertInLoopThread() { loop_->AssertInLoopThread(); }

} // namespace vraft
