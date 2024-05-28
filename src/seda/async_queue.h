#ifndef VRAFT_ASYNC_QUEUE_H_
#define VRAFT_ASYNC_QUEUE_H_

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#include "common.h"
#include "uv_wrapper.h"

namespace vraft {

class EventLoop;
void AsyncQueueCb(UvAsync *uv_async);

class AsyncQueue {
 public:
  AsyncQueue() {}
  ~AsyncQueue() {}
  AsyncQueue(const AsyncQueue &a) = delete;
  AsyncQueue &operator=(const AsyncQueue &a) = delete;

  // call in loop thread
  void Init(EventLoop *loop);
  void DoFunctor();
  void AssertInLoopThread();
  void Close();

  // call in any thread
  void Push(const Functor func);

 private:
  EventLoop *loop_;
  UvAsync uv_async_;

  std::mutex mutex_;
  std::queue<Functor> functors_;  // guarded by mutex_
};

}  // namespace vraft

#endif
