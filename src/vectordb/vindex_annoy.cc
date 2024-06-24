#include "vindex_annoy.h"

namespace vectordb {

VindexAnnoy::VindexAnnoy(VIndexParam &param, VEngineSPtr v)
    : Vindex(param), vengine_(v) {}

int32_t VindexAnnoy::GetKNN(const std::string &key, int limit,
                            std::vector<VecResult> &results) {
  return 0;
}

int32_t VindexAnnoy::GetKNN(const std::vector<float> &vec, int limit,
                            std::vector<VecResult> &results) {
  return 0;
}

int32_t VindexAnnoy::Build() { return 0; }

int32_t VindexAnnoy::Load() { return 0; }

}  // namespace vectordb
