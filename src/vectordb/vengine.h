#ifndef VECTORDB_VENGINE_H_
#define VECTORDB_VENGINE_H_

#include <memory>

namespace vectordb {

class VEngine;
using VEngineSPtr = std::shared_ptr<VEngine>;
using VEngineUPtr = std::unique_ptr<VEngine>;
using VEngineWPtr = std::weak_ptr<VEngine>;

class VEngine {
 public:
  explicit VEngine();
  virtual ~VEngine();
  VEngine(const VEngine &) = delete;
  VEngine &operator=(const VEngine &) = delete;

 private:
};

inline VEngine::VEngine() {}

inline VEngine::~VEngine() {}

}  // namespace vectordb

#endif
