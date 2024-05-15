#ifndef VRAFT_WORKER_THREAD_H_
#define VRAFT_WORKER_THREAD_H_

#include "common.h"
#include <memory>
#include <thread>

namespace vraft {

class WorkerThread;
using WorkerThreadPtr = std::shared_ptr<WorkerThread>;

class WorkerThread final {
public:
  WorkerThread();
  ~WorkerThread();
  WorkerThread(const WorkerThread &wt) = delete;
  WorkerThread &operator=(const WorkerThread &wt) = delete;

  // call in any thread
  int32_t Start();
  void Join();
  int32_t Submit(const Functor func) { return 0; }

  // call in loop thread
  std::thread::id tid() { return thread_.get_id(); }

private:
  void ThreadFunc();

private:
  std::thread thread_;
};

inline WorkerThread::WorkerThread() {}

inline WorkerThread::~WorkerThread() {}

} // namespace vraft

#endif
