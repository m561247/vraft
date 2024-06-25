#ifndef VECTORDB_METADATA_H_
#define VECTORDB_METADATA_H_

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>

#include "leveldb/db.h"
#include "nlohmann/json.hpp"
#include "vdb_common.h"

namespace vectordb {

struct Replica {
  int32_t id;
  std::string name;
  std::string path;
  uint64_t uid;

  std::string table_name;
  uint64_t table_uid;

  std::string partition_name;
  uint64_t partition_uid;

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  int32_t FromString(std::string &s);
  int32_t FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

struct Partition {
  int32_t id;
  std::string name;
  int replica_num;
  std::string path;
  uint64_t uid;

  std::unordered_map<uint64_t, ReplicaSPtr> replicas_by_uid;
  std::unordered_map<std::string, ReplicaSPtr> replicas_by_name;
};

struct Table {
  std::string name;
  int32_t dim;
  int32_t partition_num;
  int32_t replica_num;
  std::string path;
  uint64_t uid;

  std::unordered_map<uint64_t, PartitionSPtr> partitions_by_uid;
  std::unordered_map<std::string, PartitionSPtr> partitions_by_name;
};

class Metadata final {
 public:
  explicit Metadata(const std::string &path);
  ~Metadata();
  Metadata(const Metadata &) = delete;
  Metadata &operator=(const Metadata &) = delete;

 private:
  std::string path_;
  leveldb::DB *db_;
  std::unordered_map<std::string, TableSPtr> tables_;
};

inline Metadata::~Metadata() {}

}  // namespace vectordb

#endif
