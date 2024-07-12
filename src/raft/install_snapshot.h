#ifndef VRAFT_INSTALL_SNAPSHOT_H_
#define VRAFT_INSTALL_SNAPSHOT_H_

#include "allocator.h"
#include "common.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"
#include "raft_log.h"
#include "util.h"

namespace vraft {

struct InstallSnapshot {
  RaftAddr src;   // uint64_t
  RaftAddr dest;  // uint64_t
  RaftTerm term;
  uint32_t uid;

  RaftIndex last_log_index;
  RaftTerm last_log_term;

  int32_t begin_offset;
  int32_t end_offset;
  std::string data;
  bool done;

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  int32_t FromString(std::string &s);
  int32_t FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

inline int32_t InstallSnapshot::MaxBytes() { return 0; }

inline int32_t InstallSnapshot::ToString(std::string &s) { return 0; }

inline int32_t InstallSnapshot::ToString(const char *ptr, int32_t len) {
  return 0;
}

inline int32_t InstallSnapshot::FromString(std::string &s) { return 0; }

inline int32_t InstallSnapshot::FromString(const char *ptr, int32_t len) {
  return 0;
}

inline nlohmann::json InstallSnapshot::ToJson() { return 0; }

inline nlohmann::json InstallSnapshot::ToJsonTiny() { return 0; }

inline std::string InstallSnapshot::ToJsonString(bool tiny, bool one_line) {
  return 0;
}

}  // namespace vraft

#endif
