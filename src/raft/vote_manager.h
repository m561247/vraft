#ifndef VRAFT_VOTE_MANAGER_H_
#define VRAFT_VOTE_MANAGER_H_

namespace vraft {

class VoteManager final {
 public:
  VoteManager();
  ~VoteManager();
  VoteManager(const VoteManager &t) = delete;
  VoteManager &operator=(const VoteManager &t) = delete;

 private:
};

inline VoteManager::VoteManager() {}

inline VoteManager::~VoteManager() {}

}  // namespace vraft

#endif
