#ifndef VRAFT_INDEX_MANAGER_H_
#define VRAFT_INDEX_MANAGER_H_

#include <unordered_map>

#include "common.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"

namespace vraft {

struct IndexItem {
  RaftIndex next;
  RaftIndex match;
};

class IndexManager final {
 public:
  IndexManager(const std::vector<RaftAddr> &peers);
  ~IndexManager();
  IndexManager(const IndexManager &t) = delete;
  IndexManager &operator=(const IndexManager &t) = delete;

  void Reset(const std::vector<RaftAddr> &peers);
  void ResetNext(RaftIndex index);
  void ResetMatch(RaftIndex index);

 public:
  std::unordered_map<uint64_t, IndexItem> indices;

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

inline IndexManager::IndexManager(const std::vector<RaftAddr> &peers) {
  for (auto addr : peers) {
    IndexItem item;
    item.next = 0;
    item.match = 0;
    indices[addr.ToU64()] = item;
  }
}

inline IndexManager::~IndexManager() {}

inline void IndexManager::Reset(const std::vector<RaftAddr> &peers) {
  indices.clear();
  for (auto addr : peers) {
    IndexItem item;
    item.next = 0;
    item.match = 0;
    indices[addr.ToU64()] = item;
  }
}

inline void IndexManager::ResetNext(RaftIndex index) {
  for (auto &peer : indices) {
    peer.second.next = index;
  }
}

inline void IndexManager::ResetMatch(RaftIndex index) {
  for (auto &peer : indices) {
    peer.second.match = index;
  }
}

inline nlohmann::json IndexManager::ToJson() {
  nlohmann::json j;
  for (auto peer : indices) {
    RaftAddr addr(peer.first);
    j[addr.ToString()]["next"] = peer.second.next;
    j[addr.ToString()]["match"] = peer.second.match;
  }
  return j;
}

inline nlohmann::json IndexManager::ToJsonTiny() {
  nlohmann::json j;
  for (auto peer : indices) {
    RaftAddr addr(peer.first);
    j[addr.ToString()]["n"] = peer.second.next;
    j[addr.ToString()]["m"] = peer.second.match;
  }
  return j;
}

inline std::string IndexManager::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["idx_mgr"] = ToJsonTiny();
  } else {
    j["index_manager"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft

#endif
