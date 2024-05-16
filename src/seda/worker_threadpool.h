#ifndef VRAFT_WORKER_THREADPOOL_H_
#define VRAFT_WORKER_THREADPOOL_H_

#include <cstdint>
#include <vector>

#include "worker_thread.h"

namespace vraft {

class WorkerThreadPool final {
 public:
  WorkerThreadPool(int32_t thread_num);
  ~WorkerThreadPool();
  WorkerThreadPool(const WorkerThreadPool &p) = delete;
  WorkerThreadPool &operator=(const WorkerThreadPool &p) = delete;

  int32_t Start();
  WorkerThreadPtr PickThread();
  int32_t thread_num() { return thread_num_; }

 private:
  int32_t thread_num_;
  std::vector<WorkerThreadPtr> threads_;

 private:
  static int32_t current_index_;
};

inline WorkerThreadPool::WorkerThreadPool(int32_t thread_num)
    : thread_num_(thread_num) {}

inline WorkerThreadPool::~WorkerThreadPool() {}

}  // namespace vraft

#endif
