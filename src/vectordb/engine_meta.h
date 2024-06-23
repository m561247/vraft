#ifndef VECTORDB_ENGINE_META_H_
#define VECTORDB_ENGINE_META_H_

#include <memory>

namespace vectordb {

class EngineMeta;
using EngineMetaSPtr = std::shared_ptr<EngineMeta>;
using EngineMetaUPtr = std::unique_ptr<EngineMeta>;
using EngineMetaWPtr = std::weak_ptr<EngineMeta>;

class EngineMeta final {
public:
  explicit EngineMeta();
  ~EngineMeta();
  EngineMeta(const EngineMeta &) = delete;
  EngineMeta &operator=(const EngineMeta &) = delete;

private:
};

inline EngineMeta::EngineMeta() {}

inline EngineMeta::~EngineMeta() {}

} // namespace vectordb

#endif
