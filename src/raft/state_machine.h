#ifndef VRAFT_STATE_MACHINE_H_
#define VRAFT_STATE_MACHINE_H_

#include <cstdint>
#include <memory>

#include "common.h"

namespace vraft {

class LogEntry;
class StateMachine;
using StateMachineUPtr = std::unique_ptr<StateMachine>;

class StateMachine final {
 public:
  StateMachine(std::string path);
  ~StateMachine();
  StateMachine(const StateMachine &t) = delete;
  StateMachine &operator=(const StateMachine &t) = delete;
  void Init() { Restore(); }

  virtual int32_t Restore() { return 0; }
  virtual int32_t Apply(LogEntry *entry) { return 0; }
  virtual RaftIndex LastIndex() { return 0; }
  virtual RaftTerm LastTerm() { return 0; }

 private:
};

inline StateMachine::StateMachine(std::string path) {}

inline StateMachine::~StateMachine() {}

}  // namespace vraft

#endif
