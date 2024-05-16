#ifndef VRAFT_REQUEST_VOTE_REPLY_H_
#define VRAFT_REQUEST_VOTE_REPLY_H_

#include <stdint.h>

#include "allocator.h"
#include "common.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"

namespace vraft {

struct RequestVoteReply {
  RaftAddr src;   // uint64_t
  RaftAddr dest;  // uint64_t

  RaftTerm term;
  bool granted;  // uint8_t

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  bool FromString(std::string &s);
  bool FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

inline int32_t RequestVoteReply::MaxBytes() {
  return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(term) + sizeof(uint8_t);
}

inline int32_t RequestVoteReply::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr = reinterpret_cast<char *>(DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  DefaultAllocator().Free(ptr);
  return size;
}

inline int32_t RequestVoteReply::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;
  uint64_t u64 = 0;

  u64 = src.ToU64();
  EncodeFixed64(p, u64);
  p += sizeof(u64);
  size += sizeof(u64);

  u64 = dest.ToU64();
  EncodeFixed64(p, u64);
  p += sizeof(u64);
  size += sizeof(u64);

  EncodeFixed64(p, term);
  p += sizeof(term);
  size += sizeof(term);

  EncodeFixed8(p, granted);
  p += sizeof(granted);
  size += sizeof(granted);

  assert(size <= len);
  return size;
}

inline bool RequestVoteReply::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

inline bool RequestVoteReply::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  uint64_t u64 = 0;

  u64 = DecodeFixed64(p);
  src.FromU64(u64);
  p += sizeof(u64);

  u64 = DecodeFixed64(p);
  dest.FromU64(u64);
  p += sizeof(u64);

  term = DecodeFixed64(p);
  p += sizeof(term);

  granted = DecodeFixed8(p);
  p += sizeof(granted);

  return true;
}

inline nlohmann::json RequestVoteReply::ToJson() {
  nlohmann::json j;
  j["src"] = src.ToString();
  j["dest"] = dest.ToString();
  j["term"] = term;
  j["granted"] = granted;
  return j;
}

inline nlohmann::json RequestVoteReply::ToJsonTiny() {
  nlohmann::json j;
  j["src"] = src.ToString();
  j["dest"] = dest.ToString();
  j["tm"] = term;
  j["gr"] = granted;
  return j;
}

inline std::string RequestVoteReply::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["rvr"] = ToJsonTiny();
  } else {
    j["request_vote_reply"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft

#endif