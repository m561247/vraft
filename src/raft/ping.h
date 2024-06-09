#ifndef VRAFT_PING_H_
#define VRAFT_PING_H_

#include <string>

#include "allocator.h"
#include "common.h"
#include "message.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"
#include "util.h"

namespace vraft {

struct Ping {
  RaftAddr src;   // uint64_t
  RaftAddr dest;  // uint64_t
  uint32_t uid;
  std::string msg;

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  bool FromString(std::string &s);
  bool FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

inline int32_t Ping::MaxBytes() {
  return sizeof(uint64_t) + sizeof(uint64_t) + sizeof(int32_t) +
         2 * sizeof(int32_t) + msg.size();
}

inline int32_t Ping::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr = reinterpret_cast<char *>(DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  DefaultAllocator().Free(ptr);
  return size;
}

inline int32_t Ping::ToString(const char *ptr, int32_t len) {
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

  EncodeFixed32(p, uid);
  p += sizeof(uid);
  size += sizeof(uid);

  Slice sls(msg.c_str(), msg.size());
  char *p2 = EncodeString2(p, len - size, sls);
  size += (p2 - p);

  assert(size <= len);
  return size;
}

inline bool Ping::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

inline bool Ping::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  uint64_t u64 = 0;

  u64 = DecodeFixed64(p);
  src.FromU64(u64);
  p += sizeof(u64);

  u64 = DecodeFixed64(p);
  dest.FromU64(u64);
  p += sizeof(u64);

  uid = DecodeFixed32(p);
  p += sizeof(uid);

  Slice result;
  Slice input(p, len - 2 * sizeof(uint64_t));
  bool b = DecodeString(&input, &result);
  if (b) {
    msg.clear();
    msg.append(result.data(), result.size());
  }
  return b;
}

inline nlohmann::json Ping::ToJson() {
  nlohmann::json j;
  j[0]["src"] = src.ToString();
  j[0]["dest"] = dest.ToString();
  j[0]["uid"] = U32ToHexStr(uid);
  j[1]["msg"] = msg;
  return j;
}

inline nlohmann::json Ping::ToJsonTiny() {
  nlohmann::json j;
  j[0] = src.ToString();
  j[1] = dest.ToString();
  j[2] = U32ToHexStr(uid);
  j[3] = msg;
  return j;
}

inline std::string Ping::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["pi"] = ToJsonTiny();
  } else {
    j["ping"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft

#endif
