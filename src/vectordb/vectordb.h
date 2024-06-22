#ifndef VECTORDB_VECTORDB_H_
#define VECTORDB_VECTORDB_H_

#include <memory>

#include "buffer.h"
#include "common.h"
#include "server_thread.h"
#include "vdb_config.h"
#include "vengine.h"

namespace vectordb {

class VectorDB;
using VectorDBSPtr = std::shared_ptr<VectorDB>;
using VectorDBUPtr = std::unique_ptr<VectorDB>;
using VectorDBWPtr = std::weak_ptr<VectorDB>;

class VectorDB {
 public:
  explicit VectorDB(VdbConfigSPtr config);
  ~VectorDB();
  VectorDB(const VectorDB &) = delete;
  VectorDB &operator=(const VectorDB &) = delete;

  void Start() { thread_pool_.Start(); }
  void Join() { thread_pool_.Join(); }
  void Stop() { thread_pool_.Stop(); }

 private:
  void OnConnection(const vraft::TcpConnectionSPtr &conn);
  void OnMessage(const vraft::TcpConnectionSPtr &conn, vraft::Buffer *buf);

 private:
  VdbConfigSPtr config_;
  std::string path_;
  VEngineSPtr vengine_;
  vraft::ServerThreadPool thread_pool_;
};

inline VectorDB::~VectorDB() {}

}  // namespace vectordb

#endif
