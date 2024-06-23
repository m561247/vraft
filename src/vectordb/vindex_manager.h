#ifndef VECTORDB_VINDEX_MANAGER_H_
#define VECTORDB_VINDEX_MANAGER_H_

#include <map>
#include <memory>

#include "vdb_common.h"
#include "vindex.h"

namespace vectordb {

class VindexManager final {
 public:
  explicit VindexManager(VEngineSPtr v);
  ~VindexManager();
  VindexManager(const VindexManager &) = delete;
  VindexManager &operator=(const VindexManager &) = delete;

 private:
  std::string path_;
  VEngineWPtr vengine_;

  std::map<uint64_t, VindexSPtr> indices_by_time_;
  std::map<std::string, VindexSPtr> indices_by_name_;
};

inline VindexManager::~VindexManager() {}

}  // namespace vectordb

#endif
