#ifndef VRAFT_LOOP_THREAD_H_
#define VRAFT_LOOP_THREAD_H_

#include "common.h"
#include "workloop.h"
#include <memory>
#include <thread>

namespace vraft {

class LoopThread;
using LoopThreadPtr = std::shared_ptr<LoopThread>;

class LoopThread final {
public:
  LoopThread();
  ~LoopThread();
  LoopThread(const LoopThread &lt) = delete;
  LoopThread &operator=(const LoopThread &lt) = delete;

  // call in any thread
  int32_t Start();
  void Join();
  int32_t Submit(const Functor func) { return 0; }

  // call in loop thread
  std::thread::id tid() { return thread_.get_id(); }

private:
  void ThreadFunc();

private:
  WorkLoop loop_;
  std::thread thread_;
};

inline LoopThread::LoopThread() {}

inline LoopThread::~LoopThread() {}

} // namespace vraft

#endif
