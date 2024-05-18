#include "tracer.h"

#include "raft.h"

namespace vraft {

void Tracer::Init() {
  ts_ = Clock::NSec();
  snprintf(ts_buf_, sizeof(ts_buf_), "%lld", ts_);

  state0_.clear();
  state1_.clear();
  event_.clear();
}

void Tracer::PrepareEvent(EventType event_type, std::string &s) {
  if (enable_) {
    switch (event_type) {
      case kSend: {
        event_ = std::string(ts_buf_) + " event_s: " + s;
        break;
      }
      case kRecv: {
        event_ = std::string(ts_buf_) + " event_r: " + s;
        break;
      }
      default:
        assert(0);
    }
  }
}

void Tracer::PrepareState0() {
  if (enable_) {
    state0_ =
        std::string(ts_buf_) + " state_0: " + raft_->ToJsonString(true, true);
  }
}

void Tracer::PrepareState1() {
  if (enable_) {
    state1_ =
        std::string(ts_buf_) + " state_1: " + raft_->ToJsonString(true, true);
  }
}

std::string Tracer::Finish() {
  return "\n" + state0_ + "\n" + event_ + "\n" + state1_;
}
}  // namespace vraft
