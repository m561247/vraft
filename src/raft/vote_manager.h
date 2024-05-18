#ifndef VRAFT_VOTE_MANAGER_H_
#define VRAFT_VOTE_MANAGER_H_

#include <unordered_map>
#include <vector>

#include "common.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"

namespace vraft {

class VoteManager final {
 public:
  VoteManager(const std::vector<RaftAddr> &peers);
  ~VoteManager();
  VoteManager(const VoteManager &t) = delete;
  VoteManager &operator=(const VoteManager &t) = delete;

 public:
  std::unordered_map<uint64_t, bool> votes;

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

inline VoteManager::VoteManager(const std::vector<RaftAddr> &peers) {
  for (auto addr : peers) {
    votes[addr.ToU64()] = false;
  }
}

inline VoteManager::~VoteManager() {}

inline nlohmann::json VoteManager::ToJson() {
  nlohmann::json j;
  for (auto peer : votes) {
    RaftAddr addr(peer.first);
    j[addr.ToString()] = peer.second;
  }
  return j;
}

inline nlohmann::json VoteManager::ToJsonTiny() {
  nlohmann::json j;
  for (auto peer : votes) {
    RaftAddr addr(peer.first);
    j[addr.ToString()] = peer.second;
  }
  return j;
}

inline std::string VoteManager::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["vt_mgr"] = ToJsonTiny();
  } else {
    j["vote_manager"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft

#endif
