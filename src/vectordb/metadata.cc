#include "metadata.h"

#include "allocator.h"
#include "coding.h"
#include "common.h"
#include "util.h"

namespace vectordb {

int32_t Replica::MaxBytes() {
  int32_t sz = 0;
  sz += sizeof(id);
  sz += 2 * sizeof(uint32_t);
  sz += name.size();
  sz += 2 * sizeof(uint32_t);
  sz += path.size();
  sz += sizeof(uid);
  sz += 2 * sizeof(uint32_t);
  sz += table_name.size();
  sz += sizeof(table_uid);
  sz += 2 * sizeof(uint32_t);
  sz += partition_name.size();
  sz += sizeof(partition_uid);
  return sz;
}

int32_t Replica::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr =
      reinterpret_cast<char *>(vraft::DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  vraft::DefaultAllocator().Free(ptr);
  return size;
}

int32_t Replica::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  vraft::EncodeFixed32(p, id);
  p += sizeof(id);
  size += sizeof(id);

  {
    vraft::Slice sls(name.c_str(), name.size());
    char *p2 = vraft::EncodeString2(p, len - size, sls);
    size += (p2 - p);
    p = p2;
  }

  {
    vraft::Slice sls(path.c_str(), path.size());
    char *p2 = vraft::EncodeString2(p, len - size, sls);
    size += (p2 - p);
    p = p2;
  }

  vraft::EncodeFixed64(p, uid);
  p += sizeof(uid);
  size += sizeof(uid);

  {
    vraft::Slice sls(table_name.c_str(), table_name.size());
    char *p2 = vraft::EncodeString2(p, len - size, sls);
    size += (p2 - p);
    p = p2;
  }

  vraft::EncodeFixed64(p, table_uid);
  p += sizeof(table_uid);
  size += sizeof(table_uid);

  {
    vraft::Slice sls(partition_name.c_str(), partition_name.size());
    char *p2 = vraft::EncodeString2(p, len - size, sls);
    size += (p2 - p);
    p = p2;
  }

  vraft::EncodeFixed64(p, partition_uid);
  p += sizeof(partition_uid);
  size += sizeof(partition_uid);

  assert(size <= len);
  return size;
}

int32_t Replica::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

int32_t Replica::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  id = vraft::DecodeFixed32(p);
  p += sizeof(id);
  size += sizeof(id);

  {
    name.clear();
    vraft::Slice result;
    vraft::Slice input(p, len - size);
    int32_t sz = vraft::DecodeString2(&input, &result);
    if (sz > 0) {
      name.append(result.data(), result.size());
      p += sz;
      size += sz;
    }
  }

  {
    path.clear();
    vraft::Slice result;
    vraft::Slice input(p, len - size);
    int32_t sz = vraft::DecodeString2(&input, &result);
    if (sz > 0) {
      path.append(result.data(), result.size());
      p += sz;
      size += sz;
    }
  }

  uid = vraft::DecodeFixed64(p);
  p += sizeof(uid);
  size += sizeof(uid);

  {
    table_name.clear();
    vraft::Slice result;
    vraft::Slice input(p, len - size);
    int32_t sz = vraft::DecodeString2(&input, &result);
    if (sz > 0) {
      table_name.append(result.data(), result.size());
      p += sz;
      size += sz;
    }
  }

  table_uid = vraft::DecodeFixed64(p);
  p += sizeof(table_uid);
  size += sizeof(table_uid);

  {
    partition_name.clear();
    vraft::Slice result;
    vraft::Slice input(p, len - size);
    int32_t sz = vraft::DecodeString2(&input, &result);
    if (sz > 0) {
      partition_name.append(result.data(), result.size());
      p += sz;
      size += sz;
    }
  }

  partition_uid = vraft::DecodeFixed64(p);
  p += sizeof(partition_uid);
  size += sizeof(partition_uid);

  return size;
}

nlohmann::json Replica::ToJson() {
  nlohmann::json j;
  j["id"] = id;
  j["name"] = name;
  j["path"] = path;
  j["uid"] = uid;
  j["table_name"] = table_name;
  j["table_uid"] = table_uid;
  j["partition_name"] = partition_name;
  j["partition_uid"] = partition_uid;
  return j;
}

nlohmann::json Replica::ToJsonTiny() { return ToJson(); }
std::string Replica::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["table"] = ToJsonTiny();
  } else {
    j["table"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

Metadata::Metadata(const std::string &path) : path_(path) {}

}  // namespace vectordb
