#ifndef VRAFT_WORK_THREAD_H_
#define VRAFT_WORK_THREAD_H_

#include <sys/types.h>

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <mutex>
#include <thread>

#include "common.h"

namespace vraft {

class WorkThread final {
 public:
  explicit WorkThread(const std::string &name, int32_t max_queue_size);
  explicit WorkThread(const std::string &name);
  ~WorkThread();
  WorkThread(const WorkThread &t) = delete;
  WorkThread &operator=(const WorkThread &t) = delete;

  int32_t Start();
  void Stop();
  void Push(Functor func);

 private:
  void Run();

 private:
  std::string name_;
  int32_t max_queue_size_;
  bool started_;
  int32_t tid_;

  std::mutex mu_;
  std::condition_variable producer_cv_;
  std::condition_variable consumer_cv_;

  std::deque<Functor> functors_;
  std::thread thread_;
};

inline WorkThread::WorkThread(const std::string &name, int32_t max_queue_size)
    : name_(name), max_queue_size_(max_queue_size), started_(false), tid_(0) {}

inline WorkThread::WorkThread(const std::string &name)
    : name_(name), max_queue_size_(MAX_QUEUE_SIZE), started_(false), tid_(0) {}

inline WorkThread::~WorkThread() { functors_.clear(); }

class WorkThreadPool final {
 public:
  explicit WorkThreadPool();
  ~WorkThreadPool();
  WorkThreadPool(const WorkThreadPool &t) = delete;
  WorkThreadPool &operator=(const WorkThreadPool &t) = delete;

 private:
};

inline WorkThreadPool::WorkThreadPool() {}

inline WorkThreadPool::~WorkThreadPool() {}

}  // namespace vraft

#endif
