#ifndef VECTORDB_VINDEX_META_H_
#define VECTORDB_VINDEX_META_H_

#include <memory>

#include "leveldb/db.h"
#include "nlohmann/json.hpp"
#include "vindex.h"

namespace vectordb {

#define VINDEX_META_KEY "0"

class VindexMeta final {
 public:
  explicit VindexMeta(VIndexParam &param);
  ~VindexMeta();
  VindexMeta(const VindexMeta &) = delete;
  VindexMeta &operator=(const VindexMeta &) = delete;

 private:
  void Init();
  void Persist();

 private:
  leveldb::Options db_options_;
  std::shared_ptr<leveldb::DB> db_;

  VIndexParam param_;
};

inline VindexMeta::~VindexMeta() {}

}  // namespace vectordb

#endif
