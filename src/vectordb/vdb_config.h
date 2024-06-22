#ifndef VECTORDB_VDB_CONFIG_H_
#define VECTORDB_VDB_CONFIG_H_

#include <memory>

namespace vectordb {

class VdbConfig;
using VdbConfigSPtr = std::shared_ptr<VdbConfig>;
using VdbConfigUPtr = std::unique_ptr<VdbConfig>;
using VdbConfigWPtr = std::weak_ptr<VdbConfig>;

class VdbConfig final {
 public:
  explicit VdbConfig();
  ~VdbConfig();
  VdbConfig(const VdbConfig &) = delete;
  VdbConfig &operator=(const VdbConfig &) = delete;

 private:
};

inline VdbConfig::VdbConfig() {}

inline VdbConfig::~VdbConfig() {}

}  // namespace vectordb

#endif
