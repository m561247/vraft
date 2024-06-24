#ifndef VECTORDB_VINDEX_ANNOY_H_
#define VECTORDB_VINDEX_ANNOY_H_

#include <memory>

#include "annoylib.h"
#include "engine_meta.h"
#include "leveldb/db.h"
#include "nlohmann/json.hpp"
#include "vdb_common.h"
#include "vindex.h"

namespace vectordb {

class VindexAnnoy : public Vindex {
 public:
  explicit VindexAnnoy(VIndexParam &param, VEngineSPtr v);
  ~VindexAnnoy();
  VindexAnnoy(const VindexAnnoy &) = delete;
  VindexAnnoy &operator=(const VindexAnnoy &) = delete;

  int32_t GetKNN(const std::string &key, int limit,
                 std::vector<VecResult> &results) override;
  int32_t GetKNN(const std::vector<float> &vec, int limit,
                 std::vector<VecResult> &results) override;

  nlohmann::json ToJson() override;
  nlohmann::json ToJsonTiny() override;
  std::string ToJsonString(bool tiny, bool one_line) override;

 private:
  void Init();
  void MkDir();

 private:
  std::string keyid_path_;
  std::string meta_path_;
  std::string annoy_path_;

  KeyidMetaSPtr keyid_;
  VindexMetaSPtr meta_;
  VEngineWPtr vengine_;
};

inline VindexAnnoy::~VindexAnnoy() {}

}  // namespace vectordb

#endif
