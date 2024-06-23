#ifndef VECTORDB_VINDEX_MANAGER_H_
#define VECTORDB_VINDEX_MANAGER_H_

#include <map>
#include <memory>

#include "vindex.h"

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
  std::map<uint64_t, VindexSPtr> indices_by_time_;
  std::map<std::string, VindexSPtr> indices_by_name_;
};

inline VindexManager::VindexManager() {}

inline VindexManager::~VindexManager() {}

}  // namespace vectordb

#endif
