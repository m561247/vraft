#include "loop_thread.h"

namespace vraft {

int32_t LoopThread::Start() {
  thread_ = std::thread(&LoopThread::ThreadFunc, this);
  return 0;
}

void LoopThread::Join() { thread_.join(); }

void LoopThread::ThreadFunc() {}

} // namespace vraft
