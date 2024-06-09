#ifndef VRAFT_STATE_MACHINE_H_
#define VRAFT_STATE_MACHINE_H_

#include <cstdint>
#include <memory>

#include "common.h"

namespace vraft {

class LogEntry;

class StateMachine {
 public:
  StateMachine(std::string path);
  ~StateMachine();
  StateMachine(const StateMachine &t) = delete;
  StateMachine &operator=(const StateMachine &t) = delete;

  int32_t Init();

  virtual int32_t Restore() = 0;
  virtual int32_t Apply(LogEntry *entry) = 0;
  virtual RaftIndex LastIndex() = 0;
  virtual RaftTerm LastTerm() = 0;

 private:
  std::string path_;
};

}  // namespace vraft

#endif
