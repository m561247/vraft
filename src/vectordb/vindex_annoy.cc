#include "vindex_annoy.h"
#include "util.h"

namespace vectordb {

VindexAnnoy::VindexAnnoy(VIndexParam &param, VEngineSPtr v)
    : Vindex(param),
      vengine_(v),
      keyid_path_(param.path + "/keyid"),
      annoy_path_(param.path + "/annoy"),
      db_meta_path_(param.path + "/meta") {}

int32_t VindexAnnoy::GetKNN(const std::string &key, int limit,
                            std::vector<VecResult> &results) {
  return 0;
}

int32_t VindexAnnoy::GetKNN(const std::vector<float> &vec, int limit,
                            std::vector<VecResult> &results) {
  return 0;
}

void VindexAnnoy::Init() {
  if (vraft::IsDirExist(param().path)) {

  } else {

  }
}

void VindexAnnoy::MkDir() {

}

}  // namespace vectordb
