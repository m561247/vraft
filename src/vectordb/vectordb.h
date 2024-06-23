#ifndef VECTORDB_VECTORDB_H_
#define VECTORDB_VECTORDB_H_

#include <atomic>
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

  void Start();
  void Join();
  void Stop();

 private:
  void OnConnection(const vraft::TcpConnectionSPtr &conn);
  void OnMessage(const vraft::TcpConnectionSPtr &conn, vraft::Buffer *buf);
  void OnMsgVersion(const vraft::TcpConnectionSPtr &conn,
                    struct MsgVersion &msg);

 private:
  std::atomic<bool> start_;
  std::atomic<uint64_t> seqid_;
  VdbConfigSPtr config_;
  std::string path_;
  VEngineSPtr vengine_;
  vraft::ServerThreadSPtr server_thread_;
  vraft::WorkThreadPoolSPtr worker_pool_;
  std::unordered_map<uint64_t, vraft::TcpConnectionSPtr>
      requests_;  // <seqid, connection>
};

inline VectorDB::~VectorDB() {}

}  // namespace vectordb

#endif
