#ifndef VECTORDB_ENGINE_META_H_
#define VECTORDB_ENGINE_META_H_

#include <memory>

#include "leveldb/db.h"
#include "nlohmann/json.hpp"

namespace vectordb {

class EngineMeta;
using EngineMetaSPtr = std::shared_ptr<EngineMeta>;
using EngineMetaUPtr = std::unique_ptr<EngineMeta>;
using EngineMetaWPtr = std::weak_ptr<EngineMeta>;

#define ENGINE_META_KEY_DIM "dim"

class EngineMeta final {
 public:
  explicit EngineMeta(const std::string &path);
  ~EngineMeta();
  EngineMeta(const EngineMeta &) = delete;
  EngineMeta &operator=(const EngineMeta &) = delete;

  int32_t dim() const { return dim_; }
  void SetDim(int32_t dim);

 private:
  void Init();
  void PersistDim();

 private:
  std::string path_;
  leveldb::Options db_options_;
  std::shared_ptr<leveldb::DB> db_;

  int32_t dim_;
};

inline EngineMeta::~EngineMeta() {}

}  // namespace vectordb

#endif
