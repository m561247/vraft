#ifndef VRAFT_TRACER_H_
#define VRAFT_TRACER_H_

#include <string>

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

  bool Enable() { enable_ = true; }
  bool Disable() { enable_ = false; }

  void Init();
  void PrepareState0();
  void PrepareState1();
  void PrepareEvent(EventType event_type, std::string &s);
  std::string Finish();

 private:
  bool enable_;
  Raft *raft_;
  uint64_t ts_;
  char ts_buf_[32];
  std::string state0_;
  std::string state1_;
  std::string event_;
};

inline Tracer::Tracer(Raft *r, bool enable) : raft_(r), enable_(enable) {}

inline Tracer::~Tracer() {}

}  // namespace vraft

#endif
