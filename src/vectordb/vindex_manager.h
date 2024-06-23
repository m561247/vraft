#ifndef VECTORDB_VINDEX_MANAGER_H_
#define VECTORDB_VINDEX_MANAGER_H_

#include <memory>

namespace vectordb {

class VindexManager;
using VindexManagerSPtr = std::shared_ptr<VindexManager>;
using VindexManagerUPtr = std::unique_ptr<VindexManager>;
using VindexManagerWPtr = std::weak_ptr<VindexManager>;

class VindexManager final {
 public:
  explicit VindexManager();
  ~VindexManager();
  VindexManager(const VindexManager &) = delete;
  VindexManager &operator=(const VindexManager &) = delete;

 private:
};

inline VindexManager::VindexManager() {}

inline VindexManager::~VindexManager() {}

}  // namespace vectordb

#endif
