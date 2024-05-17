#ifndef VRAFT_APPEND_ENTRIES_REPLY_H_
#define VRAFT_APPEND_ENTRIES_REPLY_H_

#include <stdint.h>

#include "allocator.h"
#include "common.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"

namespace vraft {

struct AppendEntriesReply {
  RaftAddr src;   // uint64_t
  RaftAddr dest;  // uint64_t
  RaftTerm term;

  bool success;              // uint8_t
  RaftIndex last_log_index;  // to speed up

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  bool FromString(std::string &s);
  bool FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

inline int32_t AppendEntriesReply::MaxBytes() {
  return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(term) + sizeof(uint8_t) +
         sizeof(last_log_index);
}

inline int32_t AppendEntriesReply::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr = reinterpret_cast<char *>(DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  DefaultAllocator().Free(ptr);
  return size;
}

inline int32_t AppendEntriesReply::ToString(const char *ptr, int32_t len) {
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

  EncodeFixed8(p, success);
  p += sizeof(success);
  size += sizeof(uint8_t);

  EncodeFixed32(p, last_log_index);
  p += sizeof(last_log_index);
  size += sizeof(last_log_index);

  assert(size <= len);
  return size;
}

inline bool AppendEntriesReply::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

inline bool AppendEntriesReply::FromString(const char *ptr, int32_t len) {
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

  success = DecodeFixed8(p);
  p += sizeof(uint8_t);

  last_log_index = DecodeFixed32(p);
  p += sizeof(last_log_index);

  return true;
}

inline nlohmann::json AppendEntriesReply::ToJson() {
  nlohmann::json j;
  j["src"] = src.ToString();
  j["dest"] = dest.ToString();
  j["term"] = term;
  j["success"] = success;
  j["last_log_index"] = last_log_index;
  return j;
}

inline nlohmann::json AppendEntriesReply::ToJsonTiny() {
  nlohmann::json j;
  j["src"] = src.ToString();
  j["dst"] = dest.ToString();
  j["tm"] = term;
  j["suc"] = success;
  j["lidx"] = last_log_index;
  return j;
}

inline std::string AppendEntriesReply::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["aer"] = ToJsonTiny();
  } else {
    j["append_entries_reply"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft

#endif
