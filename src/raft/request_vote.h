#ifndef VRAFT_REQUEST_VOTE_H_
#define VRAFT_REQUEST_VOTE_H_

#include <stdint.h>

#include "common.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"

namespace vraft {

struct RequestVote {
  RaftAddr src;   // uint64_t
  RaftAddr dest;  // uint64_t

  uint64_t id;
  RaftTerm term;
  RaftTerm last_log_term;
  RaftIndex last_log_index;

  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  bool FromString(std::string &s);
  bool FromString(const char *ptr, int32_t len);
  std::string ToJson(bool one_line = true);
};

int32_t ToString(std::string &s);
int32_t ToString(const char *ptr, int32_t len);
bool FromString(std::string &s);
bool FromString(const char *ptr, int32_t len);
std::string ToJson(bool one_line = true);

}  // namespace vraft

#endif
