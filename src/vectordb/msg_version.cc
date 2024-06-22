#include "msg_version.h"

namespace vectordb {
int32_t MsgVersion::MaxBytes() { return sizeof(uint64_t) + sizeof(uint64_t); }

int32_t MsgVersion::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr =
      reinterpret_cast<char *>(vraft::DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  vraft::DefaultAllocator().Free(ptr);
  return size;
}

int32_t MsgVersion::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;
  uint64_t u64 = 0;

  u64 = src.ToU64();
  vraft::EncodeFixed64(p, u64);
  p += sizeof(u64);
  size += sizeof(u64);

  u64 = dest.ToU64();
  vraft::EncodeFixed64(p, u64);
  p += sizeof(u64);
  size += sizeof(u64);

  assert(size <= len);
  return size;
}

int32_t MsgVersion::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

int32_t MsgVersion::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  uint64_t u64 = 0;
  int32_t size = 0;

  u64 = vraft::DecodeFixed64(p);
  src.FromU64(u64);
  p += sizeof(u64);
  size += sizeof(u64);

  u64 = vraft::DecodeFixed64(p);
  dest.FromU64(u64);
  p += sizeof(u64);
  size += sizeof(u64);

  return size;
}

nlohmann::json MsgVersion::ToJson() {
  nlohmann::json j;
  j[0]["src"] = src.ToString();
  j[0]["dest"] = dest.ToString();
  return j;
}

nlohmann::json MsgVersion::ToJsonTiny() {
  nlohmann::json j;
  j["src"] = src.ToString();
  j["dst"] = dest.ToString();
  return j;
}

std::string MsgVersion::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["ver"] = ToJsonTiny();
  } else {
    j["version"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vectordb
