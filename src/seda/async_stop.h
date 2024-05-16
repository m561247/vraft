#ifndef VRAFT_ASYNC_STOP_H_
#define VRAFT_ASYNC_STOP_H_

#include "common.h"
#include "uv_wrapper.h"

namespace vraft {

class EventLoop;
void StopLoop(UvAsync *uv_async);

class AsyncStop final {
 public:
  AsyncStop();
  ~AsyncStop();
  AsyncStop(const AsyncStop &a) = delete;
  AsyncStop &operator=(const AsyncStop &a) = delete;

  // call in loop thread
  void Init(EventLoop *loop);
  void AssertInLoopThread();

  // call in any thread
  void Notify();

 private:
  EventLoop *loop_;
  UvAsync uv_async_;

  friend void StopLoop(UvAsync *uv_async);
};

inline AsyncStop::AsyncStop() {}

inline AsyncStop::~AsyncStop() {}

}  // namespace vraft

#endif
