#ifndef VECTORDB_VDB_ENGINE_H_
#define VECTORDB_VDB_ENGINE_H_

#include <atomic>
#include <memory>

#include "buffer.h"
#include "common.h"
#include "metadata.h"
#include "nlohmann/json.hpp"
#include "server_thread.h"
#include "unordered_map"
#include "vdb_common.h"
#include "vdb_config.h"
#include "vengine.h"

namespace vectordb {

struct AddTableParam {
  std::string name;
  int32_t partition_num;
  int32_t replica_num;
  int32_t dim;
};

class VdbEngine final {
 public:
  explicit VdbEngine(const std::string &path);
  ~VdbEngine();
  VdbEngine(const VdbEngine &) = delete;
  VdbEngine &operator=(const VdbEngine &) = delete;

  int32_t AddTable(AddTableParam param);
  int32_t Put(const std::string &table, const std::string &key, VecValue &vv);
  int32_t Get(const std::string &table, const std::string &key, VecObj &vo);
  int32_t Delete(const std::string &table, const std::string &key);
  int32_t Load(const std::string &table, const std::string &file_path);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);

 private:
  void Init();
  void MkDir();
  int32_t CreateVEngine(ReplicaSPtr replica);
  VEngineSPtr GetVEngine(const std::string &table, const std::string &key);

 private:
  std::string path_;
  std::string meta_path_;
  std::string data_path_;

  MetadataSPtr meta_;
  std::unordered_map<uint64_t, VEngineSPtr> engines_;
};

inline VdbEngine::~VdbEngine() {}

}  // namespace vectordb

#endif
