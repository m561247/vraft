#include "metadata.h"

#include "allocator.h"
#include "coding.h"
#include "common.h"
#include "util.h"

namespace vectordb {

std::string PartitionName(const std::string &table_name, int32_t partition_id) {
  char buf[256];
  snprintf(buf, sizeof(buf), "%s#%d", table_name.c_str(), partition_id);
  return std::string(buf);
}

std::string ReplicaName(const std::string &partition_name, int32_t replica_id) {
  char buf[256];
  snprintf(buf, sizeof(buf), "%s#%d", partition_name.c_str(), replica_id);
  return std::string(buf);
}

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
    j["replica"] = ToJsonTiny();
  } else {
    j["replica"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

int32_t Partition::MaxBytes() {
  int32_t sz = 0;
  sz += sizeof(id);
  sz += 2 * sizeof(uint32_t);
  sz += name.size();
  sz += 2 * sizeof(uint32_t);
  sz += path.size();
  sz += sizeof(replica_num);
  sz += sizeof(uid);

  // replicas_by_uid.size()
  sz += sizeof(uint32_t);

  // uids
  for (auto item : replicas_by_uid) {
    sz += sizeof(item.first);
  }

  // replicas_by_name.size()
  sz += sizeof(uint32_t);

  // names
  for (auto item : replicas_by_name) {
    sz += 2 * sizeof(uint32_t);
    sz += item.first.size();
  }

  return sz;
}

int32_t Partition::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr =
      reinterpret_cast<char *>(vraft::DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  vraft::DefaultAllocator().Free(ptr);
  return size;
}

int32_t Partition::ToString(const char *ptr, int32_t len) {
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

  vraft::EncodeFixed32(p, replica_num);
  p += sizeof(replica_num);
  size += sizeof(replica_num);

  vraft::EncodeFixed64(p, uid);
  p += sizeof(uid);
  size += sizeof(uid);

  // uids
  for (auto item : replicas_by_uid) {
    uint64_t u64 = item.first;
    vraft::EncodeFixed64(p, u64);
    p += sizeof(u64);
    size += sizeof(u64);
  }

  // names
  for (auto item : replicas_by_name) {
    vraft::Slice sls(item.first.c_str(), item.first.size());
    char *p2 = vraft::EncodeString2(p, len - size, sls);
    size += (p2 - p);
    p = p2;
  }

  assert(size <= len);
  return size;
}

int32_t Partition::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

int32_t Partition::FromString(const char *ptr, int32_t len) {
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

  replica_num = vraft::DecodeFixed32(p);
  p += sizeof(replica_num);
  size += sizeof(replica_num);

  uid = vraft::DecodeFixed64(p);
  p += sizeof(uid);
  size += sizeof(uid);

  // uids
  for (int32_t i = 0; i < replica_num; ++i) {
    uint64_t u64 = vraft::DecodeFixed64(p);
    p += sizeof(u64);
    size += sizeof(u64);
    replicas_by_uid[u64] = nullptr;
  }

  // names
  for (int32_t i = 0; i < replica_num; ++i) {
    std::string replica_name;
    vraft::Slice result;
    vraft::Slice input(p, len - size);
    int32_t sz = vraft::DecodeString2(&input, &result);
    if (sz > 0) {
      replica_name.append(result.data(), result.size());
      p += sz;
      size += sz;
    }
    replicas_by_name[replica_name] = nullptr;
  }

  return size;
}

nlohmann::json Partition::ToJson() {
  nlohmann::json j;
  j["id"] = id;
  j["name"] = name;
  j["path"] = path;
  j["replica_num"] = replica_num;
  j["uid"] = uid;

  int32_t i = 0;
  for (auto item : replicas_by_uid) {
    j["replica_uids"][i++] = item.first;
  }

  i = 0;
  for (auto item : replicas_by_name) {
    j["replica_names"][i++] = item.first;
  }

  return j;
}

nlohmann::json Partition::ToJsonTiny() { return ToJson(); }

std::string Partition::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["partition"] = ToJsonTiny();
  } else {
    j["partition"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

int32_t Table::MaxBytes() {
  int32_t sz = 0;
  sz += 2 * sizeof(uint32_t);
  sz += name.size();
  sz += 2 * sizeof(uint32_t);
  sz += path.size();
  sz += sizeof(partition_num);
  sz += sizeof(replica_num);
  sz += sizeof(dim);
  sz += sizeof(uid);

  // partitions_by_uid.size()
  sz += sizeof(uint32_t);

  // uids
  for (auto item : partitions_by_uid) {
    sz += sizeof(item.first);
  }

  // replicas_by_name.size()
  sz += sizeof(uint32_t);

  // names
  for (auto item : partitions_by_name) {
    sz += 2 * sizeof(uint32_t);
    sz += item.first.size();
  }

  return sz;
}

int32_t Table::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr =
      reinterpret_cast<char *>(vraft::DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  vraft::DefaultAllocator().Free(ptr);
  return size;
}

int32_t Table::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

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

  vraft::EncodeFixed32(p, partition_num);
  p += sizeof(partition_num);
  size += sizeof(partition_num);

  vraft::EncodeFixed32(p, replica_num);
  p += sizeof(replica_num);
  size += sizeof(replica_num);

  vraft::EncodeFixed32(p, dim);
  p += sizeof(dim);
  size += sizeof(dim);

  vraft::EncodeFixed64(p, uid);
  p += sizeof(uid);
  size += sizeof(uid);

  // uids
  for (auto item : partitions_by_uid) {
    uint64_t u64 = item.first;
    vraft::EncodeFixed64(p, u64);
    p += sizeof(u64);
    size += sizeof(u64);
  }

  // names
  for (auto item : partitions_by_name) {
    vraft::Slice sls(item.first.c_str(), item.first.size());
    char *p2 = vraft::EncodeString2(p, len - size, sls);
    size += (p2 - p);
    p = p2;
  }

  assert(size <= len);
  return size;
}

int32_t Table::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

int32_t Table::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

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

  partition_num = vraft::DecodeFixed32(p);
  p += sizeof(partition_num);
  size += sizeof(partition_num);

  replica_num = vraft::DecodeFixed32(p);
  p += sizeof(replica_num);
  size += sizeof(replica_num);

  dim = vraft::DecodeFixed32(p);
  p += sizeof(dim);
  size += sizeof(dim);

  uid = vraft::DecodeFixed64(p);
  p += sizeof(uid);
  size += sizeof(uid);

  // uids
  for (int32_t i = 0; i < partition_num; ++i) {
    uint64_t u64 = vraft::DecodeFixed64(p);
    p += sizeof(u64);
    size += sizeof(u64);
    partitions_by_uid[u64] = nullptr;
  }

  // names
  for (int32_t i = 0; i < partition_num; ++i) {
    std::string partition_name;
    vraft::Slice result;
    vraft::Slice input(p, len - size);
    int32_t sz = vraft::DecodeString2(&input, &result);
    if (sz > 0) {
      partition_name.append(result.data(), result.size());
      p += sz;
      size += sz;
    }
    partitions_by_name[partition_name] = nullptr;
  }

  return size;
}

nlohmann::json Table::ToJson() {
  nlohmann::json j;
  j["name"] = name;
  j["path"] = path;
  j["partition_num"] = partition_num;
  j["replica_num"] = replica_num;
  j["dim"] = dim;
  j["uid"] = uid;

  int32_t i = 0;
  for (auto item : partitions_by_uid) {
    j["replica_uids"][i++] = item.first;
  }

  i = 0;
  for (auto item : partitions_by_name) {
    j["replica_names"][i++] = item.first;
  }

  return j;
}

nlohmann::json Table::ToJsonTiny() { return ToJson(); }

std::string Table::ToJsonString(bool tiny, bool one_line) {
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
