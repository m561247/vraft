#include "work_thread.h"

#include <sys/types.h>
#include <unistd.h>

namespace vraft {

int32_t WorkThread::Start() {
  started_ = true;
  thread_ = std::thread(std::bind(&WorkThread::Run, this));
  thread_.detach();
  return 0;
}

void WorkThread::Stop() {
  bool* started = &started_;
  Push([started] { *started = false; });
}

void WorkThread::Push(Functor func) {
  {  // critical section
     // lock
    std::unique_lock<std::mutex> ulk(mu_);

    // wait for condition
    std::deque<Functor>* functors = &functors_;
    int32_t max_queue_size = max_queue_size_;
    producer_cv_.wait(ulk, [functors, max_queue_size] {
      return (functors->size() < static_cast<size_t>(max_queue_size));
    });

    // assert condition
    assert(functors_.size() < static_cast<size_t>(max_queue_size_));

    // push item
    functors_.push_back(func);
  }

  // notify consumer
  consumer_cv_.notify_one();
}

void WorkThread::Run() {
  tid_ = gettid();
  while (started_) {
    std::deque<Functor> tmp_functors;
    {  // critical section
       // lock
      std::unique_lock<std::mutex> ulk(mu_);

      // wait for condition
      std::deque<Functor>* functors = &functors_;
      consumer_cv_.wait(ulk, [functors] { return !(functors->empty()); });

      // assert condition
      assert(!functors_.empty());

      // get items
      std::swap(tmp_functors, functors_);
    }

    // notify producers
    producer_cv_.notify_all();

    // do jobs out of critical section
    assert(!tmp_functors.empty());
    for (auto& item : tmp_functors) {
      item();
    }
    tmp_functors.clear();
  }
}

}  // namespace vraft
