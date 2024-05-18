#ifndef VRAFT_CONFIG_MANAGER_H_
#define VRAFT_CONFIG_MANAGER_H_

#include <vector>

#include "common.h"
#include "nlohmann/json.hpp"
#include "raft_addr.h"

namespace vraft {

class RaftConfig final {
 public:
  RaftAddr me;
  std::vector<RaftAddr> peers;

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);

 public:
};

inline nlohmann::json RaftConfig::ToJson() {
  nlohmann::json j;
  j["me"] = me.ToString();
  int32_t i = 0;
  for (auto peer : peers) {
    j["peers"][i++] = peer.ToString();
  }
  return j;
}

inline nlohmann::json RaftConfig::ToJsonTiny() {
  nlohmann::json j;
  j["me"] = me.ToString();
  int32_t i = 0;
  for (auto peer : peers) {
    j["peers"][i++] = peer.ToString();
  }
  return j;
}

inline std::string RaftConfig::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["rc"] = ToJsonTiny();
  } else {
    j["raft_config"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

class ConfigManager final {
 public:
  ConfigManager(const RaftConfig& config);
  ~ConfigManager();
  ConfigManager(const ConfigManager& t) = delete;
  ConfigManager& operator=(const ConfigManager& t) = delete;

  RaftConfig& Current();

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);

 private:
  RaftConfig current_;
};

inline ConfigManager::ConfigManager(const RaftConfig& config)
    : current_(config) {}

inline ConfigManager::~ConfigManager() {}

inline RaftConfig& ConfigManager::Current() { return current_; }

inline nlohmann::json ConfigManager::ToJson() {
  nlohmann::json j;
  j = current_.ToJson();
  return j;
}

inline nlohmann::json ConfigManager::ToJsonTiny() {
  nlohmann::json j;
  j = current_.ToJsonTiny();
  return j;
}

inline std::string ConfigManager::ToJsonString(bool tiny, bool one_line) {
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
