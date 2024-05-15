#ifndef VRAFT_LOOP_THREADPOOL_H_
#define VRAFT_LOOP_THREADPOOL_H_

#include "loop_thread.h"
#include <cstdint>
#include <vector>

namespace vraft {

class LoopThreadPool final {
public:
  LoopThreadPool(int32_t thread_num);
  ~LoopThreadPool();
  LoopThreadPool(const LoopThreadPool &p) = delete;
  LoopThreadPool &operator=(const LoopThreadPool &p) = delete;

  int32_t Start();
  LoopThreadPtr PickThread();
  int32_t thread_num() { return thread_num_; }

private:
  int32_t thread_num_;
  std::vector<LoopThreadPtr> threads_;

private:
  static int32_t current_index_;
};

inline LoopThreadPool::LoopThreadPool(int32_t thread_num)
    : thread_num_(thread_num) {}

inline LoopThreadPool::~LoopThreadPool() {}

} // namespace vraft

#endif
