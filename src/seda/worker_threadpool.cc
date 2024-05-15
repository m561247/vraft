#include "worker_threadpool.h"

namespace vraft {

int32_t WorkerThreadPool::Start() {
  for (int32_t i = 0; i < thread_num_; ++i) {
    char loop_name[32];
    snprintf(loop_name, sizeof(loop_name), "loop_%d", i);
    WorkerThreadPtr ptr = std::make_shared<WorkerThread>();
    threads_.push_back(ptr);
    ptr->Start();
  }
  return 0;
}

WorkerThreadPtr WorkerThreadPool::PickThread() {
  current_index_ = (current_index_ + 1) % threads_.size();
  WorkerThreadPtr ptr = threads_[current_index_];
  return ptr;
}

int32_t WorkerThreadPool::current_index_ = 0;

} // namespace vraft
