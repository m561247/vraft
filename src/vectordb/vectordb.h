#ifndef VECTORDB_VECTORDB_H_
#define VECTORDB_VECTORDB_H_

#include <memory>

#include "common.h"
#include "vdb_config.h"
#include "vengine.h"

namespace vectordb {

class VectorDB;
using VectorDBSPtr = std::shared_ptr<VectorDB>;
using VectorDBUPtr = std::unique_ptr<VectorDB>;
using VectorDBWPtr = std::weak_ptr<VectorDB>;

class VectorDB {
 public:
  explicit VectorDB(const std::string &path, VdbConfigSPtr config);
  ~VectorDB();
  VectorDB(const VectorDB &) = delete;
  VectorDB &operator=(const VectorDB &) = delete;

  int32_t Start();
  void Stop();

 private:
  std::string path_;
  VdbConfigSPtr config_;
  VEngineSPtr vengine_;
  vraft::ServerThreadSPtr server_thread_;
};

inline VectorDB::VectorDB(const std::string &path, VdbConfigSPtr config)
    : path_(path), config_(config) {}

inline VectorDB::~VectorDB() {}

}  // namespace vectordb

#endif
