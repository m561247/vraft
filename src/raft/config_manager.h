#ifndef VRAFT_CONFIG_MANAGER_H_
#define VRAFT_CONFIG_MANAGER_H_

#include <vector>

#include "raft_addr.h"

namespace vraft {

class RaftConfig final {
 public:
  RaftAddr me;
  std::vector<RaftAddr> peers;

 public:
};

class ConfigManager final {
 public:
  ConfigManager(const RaftConfig& config);
  ~ConfigManager();
  ConfigManager(const ConfigManager& t) = delete;
  ConfigManager& operator=(const ConfigManager& t) = delete;

  RaftConfig& Current();

 private:
  RaftConfig current_;
};

inline ConfigManager::ConfigManager(const RaftConfig& config)
    : current_(config) {}

inline ConfigManager::~ConfigManager() {}

inline RaftConfig& ConfigManager::Current() { return current_; }

}  // namespace vraft

#endif
