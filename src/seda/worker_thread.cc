#include "worker_thread.h"

namespace vraft {

int32_t WorkerThread::Start() {
  thread_ = std::thread(&WorkerThread::ThreadFunc, this);
  return 0;
}

void WorkerThread::Join() { thread_.join(); }

void WorkerThread::ThreadFunc() {}

}  // namespace vraft
