#ifndef VECTORDB_VENGINE_H_
#define VECTORDB_VENGINE_H_

#include <memory>

#include "engine_meta.h"
#include "leveldb/db.h"
#include "nlohmann/json.hpp"

namespace vectordb {

struct Vec {
  std::vector<float> data;
};

class VecObj {
  std::string key;
  Vec vec;
  std::string attach_value;
};

class VecResult {
  std::string key;
  std::string attach_value;
  float distance;
};

class VEngine;
using VEngineSPtr = std::shared_ptr<VEngine>;
using VEngineUPtr = std::unique_ptr<VEngine>;
using VEngineWPtr = std::weak_ptr<VEngine>;

enum VIndexType {
  kIndexAnnoy = 0,
  kIndexNum,
};

class VEngine final {
 public:
  explicit VEngine(const std::string &path, int32_t dim);
  ~VEngine();
  VEngine(const VEngine &) = delete;
  VEngine &operator=(const VEngine &) = delete;

  std::string path() const { return path_; }
  int32_t Dim() const;

  int32_t Put(const std::string &key, const VecObj &vo) { return 0; }
  int32_t Get(const std::string &key, VecObj &vo) const { return 0; }
  int32_t Delete(const std::string &key) { return 0; }

  bool HasIndex() const { return 0; }
  int32_t AddIndex(VIndexType type) { return 0; }
  int32_t GetKNN(const std::string &key, int limit,
                 std::vector<VecResult> &results,
                 const std::string &index_name) {
    return 0;
  }
  int32_t GetKNN(const std::vector<float> &vec, int limit,
                 std::vector<VecResult> &results,
                 const std::string &index_name) {
    return 0;
  }

 private:
  void Init();

 private:
  std::string path_;
  std::string meta_path_;
  std::string data_path_;
  std::string index_path_;

  leveldb::Options db_options_;
  std::shared_ptr<leveldb::DB> db_;
  EngineMetaSPtr meta_;
};

inline VEngine::~VEngine() {}

}  // namespace vectordb

#endif
