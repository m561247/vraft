#ifndef VECTORDB_VDB_ENGINE_H_
#define VECTORDB_VDB_ENGINE_H_

#include <atomic>
#include <memory>

#include "buffer.h"
#include "common.h"
#include "metadata.h"
#include "server_thread.h"
#include "unordered_map"
#include "vdb_config.h"
#include "vengine.h"

namespace vectordb {

class VdbEngine final {
 public:
  explicit VdbEngine(const std::string &path);
  ~VdbEngine();
  VdbEngine(const VdbEngine &) = delete;
  VdbEngine &operator=(const VdbEngine &) = delete;

 private:
  void Init();
  void MkDir();

 private:
  std::string path_;
  std::string meta_path_;
  std::string data_path_;

  MetadataSPtr meta_;
  std::unordered_map<uint64_t, VEngineSPtr> engines_;
};

inline VdbEngine::~VdbEngine() {}

}  // namespace vectordb

#endif
