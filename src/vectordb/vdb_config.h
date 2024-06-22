#ifndef VECTORDB_VDB_CONFIG_H_
#define VECTORDB_VDB_CONFIG_H_

#include <iostream>
#include <memory>
#include <mutex>

namespace vectordb {

class VdbConfig;
using VdbConfigSPtr = std::shared_ptr<VdbConfig>;
using VdbConfigUPtr = std::unique_ptr<VdbConfig>;
using VdbConfigWPtr = std::weak_ptr<VdbConfig>;

class VdbConfig final {
 public:
  explicit VdbConfig();
  ~VdbConfig();
  VdbConfig(const VdbConfig &) = delete;
  VdbConfig &operator=(const VdbConfig &) = delete;

 private:
};

inline VdbConfig::VdbConfig() {}

inline VdbConfig::~VdbConfig() {}

class ConfigSingleton {
 public:
  ConfigSingleton(const ConfigSingleton &) = delete;
  ConfigSingleton &operator=(const ConfigSingleton &) = delete;

  static VdbConfigSPtr GetInstance() {
    static std::once_flag init_flag;
    std::call_once(init_flag, []() { instance_.reset(new VdbConfig); });

    VdbConfigSPtr sptr;
    {
      std::unique_lock<std::mutex> ulk(mu_);
      sptr = instance_;
    }
    return sptr;
  }

 private:
  ConfigSingleton() {}
  ~ConfigSingleton() {}

  static std::mutex mu_;
  static VdbConfigSPtr instance_;
};

}  // namespace vectordb

#endif
