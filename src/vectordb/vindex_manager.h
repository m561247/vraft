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

  bool HasIndex() const;
  int32_t Add(VindexSPtr index);
  int32_t Del(uint64_t timestamp);
  VindexSPtr Get(uint64_t timestamp);
  VindexSPtr GetNewest();

 private:
  VEngineWPtr vengine_;
  std::string path_;

  std::map<uint64_t, VindexSPtr> indices_;
};

inline VindexManager::~VindexManager() {}

}  // namespace vectordb

#endif
