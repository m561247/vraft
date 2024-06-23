#ifndef VECTORDB_VINDEX_H_
#define VECTORDB_VINDEX_H_

#include <memory>
#include <string>
#include <vector>

#include "vdb_common.h"

namespace vectordb {

enum VIndexType {
  kIndexAnnoy = 0,
  kIndexNum,
};

enum DistanceType {

};

struct VIndexParam {
  std::string path;
  int dim;
  VIndexType index_type;
  DistanceType distance_type;
};

class VecResult {
  std::string key;
  std::string attach_value;
  float distance;
};

class Vindex final {
 public:
  explicit Vindex(const VIndexParam &param);
  ~Vindex();
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

}  // namespace vectordb

#endif
