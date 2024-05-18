#include "tracer.h"

#include "raft.h"

namespace vraft {

void Tracer::Init() {
  ts_ = Clock::NSec();
  snprintf(ts_buf_, sizeof(ts_buf_), "0x%llX", ts_);

  state0_.clear();
  state1_.clear();
  events_.clear();
}

std::string Tracer::TimeStampStr() {
  if (timestamp_) {
    return " " + Clock::NSecStr();
  } else {
    return "";
  }
}

void Tracer::PrepareEvent(EventType event_type, std::string s) {
  if (enable_) {
    switch (event_type) {
      case kSend: {
        events_.push_back(std::string(ts_buf_) + TimeStampStr() +
                          " event_send : " + s);
        break;
      }
      case kRecv: {
        events_.push_back(std::string(ts_buf_) + TimeStampStr() +
                          " event_recv : " + s);
        break;
      }
      default:
        assert(0);
    }
  }
}

void Tracer::PrepareState0() {
  if (enable_) {
    state0_ = std::string(ts_buf_) + TimeStampStr() +
              " state_begin: " + raft_->ToJsonString(true, true);
  }
}

void Tracer::PrepareState1() {
  if (enable_) {
    state1_ = std::string(ts_buf_) + TimeStampStr() +
              " state_end  : " + raft_->ToJsonString(true, true);
  }
}

std::string Tracer::Finish() {
  std::string s;
  if (enable_) {
    s.append("\n" + state0_);
    for (auto &e : events_) {
      s.append("\n" + e);
    }
    s.append("\n" + state1_);
  }
  return s;
}
}  // namespace vraft
