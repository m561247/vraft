#ifndef VRAFT_INDEX_MANAGER_H_
#define VRAFT_INDEX_MANAGER_H_

namespace vraft {

class IndexManager final {
 public:
  IndexManager();
  ~IndexManager();
  IndexManager(const IndexManager &t) = delete;
  IndexManager &operator=(const IndexManager &t) = delete;

 private:
};

inline IndexManager::IndexManager() {}

inline IndexManager::~IndexManager() {}

}  // namespace vraft

#endif
