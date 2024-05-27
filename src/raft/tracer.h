#ifndef VRAFT_TRACER_H_
#define VRAFT_TRACER_H_

#include <string>
#include <vector>

#include "clock.h"

namespace vraft {

class Raft;

enum EventType {
  kSend = 0,
  kRecv,
  kEventTypeNum,
};

class Tracer final {
 public:
  Tracer(Raft *r, bool enable);
  ~Tracer();
  Tracer(const Tracer &t) = delete;
  Tracer &operator=(const Tracer &t) = delete;

  void Enable() { enable_ = true; }
  void Disable() { enable_ = false; }

  void EnableTimeStamp() { timestamp_ = true; }
  void DisableTimeStamp() { timestamp_ = false; }

  void PrepareState0();
  void PrepareState1();
  void PrepareEvent(EventType event_type, std::string s);
  std::string Finish();

 private:
  void Init();
  std::string TimeStampStr();

 private:
  bool enable_;
  bool timestamp_;

  Raft *raft_;
  uint64_t ts_;
  char ts_buf_[32];
  std::string state0_;
  std::string state1_;
  std::vector<std::string> events_;
};

inline Tracer::Tracer(Raft *r, bool enable)
    : enable_(enable), timestamp_(false), raft_(r) {
  Init();
}

inline Tracer::~Tracer() {}

}  // namespace vraft

#endif
