#include "vindex_manager.h"

#include <map>

#include "vengine.h"

namespace vectordb {

VindexManager::VindexManager() {}

bool VindexManager::HasIndex() const { return indices_.size() > 0; }

int32_t VindexManager::Add(VindexSPtr index) {
  indices_[index->param().timestamp] = index;
  return 0;
}

int32_t VindexManager::Del(uint64_t timestamp) {
  indices_.erase(timestamp);
  return 0;
}

VindexSPtr VindexManager::Get(uint64_t timestamp) {
  VindexSPtr sptr = nullptr;
  auto it = indices_.find(timestamp);
  if (it != indices_.end()) {
    sptr = it->second;
  }
  return sptr;
}

VindexSPtr VindexManager::GetNewest() {
  VindexSPtr sptr = nullptr;
  if (indices_.size() > 0) {
    auto it = indices_.rbegin();
    sptr = it->second;
  }
  return sptr;
}

}  // namespace vectordb
