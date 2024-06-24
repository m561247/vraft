#ifndef VECTORDB_VINDEX_H_
#define VECTORDB_VINDEX_H_

#include <memory>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"
#include "vdb_common.h"

namespace vectordb {

enum VIndexType {
  kIndexAnnoy = 0,
  kIndexNum,
};

VIndexType U82VIndexType(uint8_t u8) {
  switch (u8) {
    case 0:
      return kIndexAnnoy;
    default:
      assert(0);
  }
}

inline const char *VIndexType2Str(VIndexType index_type) {
  switch (index_type) {
    case kIndexAnnoy:
      return "IndexAnnoy";

    default:
      return "UnknownIndexType";
  }
}

enum DistanceType {
  kCosine = 0,
  kInnerProduct,
  kEuclidean,
};

DistanceType U82DistanceType(uint8_t u8) {
  switch (u8) {
    case 0:
      return kCosine;
    case 1:
      return kInnerProduct;
    case 2:
      return kEuclidean;
    default:
      assert(0);
  }
}

inline const char *DistanceType2Str(DistanceType distance_type) {
  switch (distance_type) {
    case kCosine:
      return "Cosine";
    case kInnerProduct:
      return "InnerProduct";
    case kEuclidean:
      return "Euclidean";
    default:
      return "UnknownDistanceType";
  }
}

struct VIndexParam {
  std::string path;
  uint64_t timestamp;
  int32_t dim;
  VIndexType index_type;       // uint8_t
  DistanceType distance_type;  // uint8_t
  int32_t annoy_tree_num;

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  int32_t FromString(std::string &s);
  int32_t FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

class VecResult {
  std::string key;
  std::string attach_value;
  float distance;
};

class Vindex {
 public:
  explicit Vindex(const VIndexParam &param);
  virtual ~Vindex();
  Vindex(const Vindex &) = delete;
  Vindex &operator=(const Vindex &) = delete;

  VIndexParam param() const { return param_; }

  virtual int32_t GetKNN(const std::string &key, int limit,
                         std::vector<VecResult> &results) = 0;
  virtual int32_t GetKNN(const std::vector<float> &vec, int limit,
                         std::vector<VecResult> &results) = 0;

 private:
  VIndexParam param_;
};

inline Vindex::Vindex(const VIndexParam &param) {}

inline Vindex::~Vindex() {}

VindexSPtr Create(VIndexType index_type, const std::string &path, VEngineSPtr v,
                  VIndexParam &param);

}  // namespace vectordb

#endif
