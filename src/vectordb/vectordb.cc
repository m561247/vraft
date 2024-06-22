#include "vectordb.h"

#include <functional>

#include "annoy_db.h"
#include "server_thread.h"

namespace vectordb {

VectorDB::VectorDB(VdbConfigSPtr config)
    : config_(config),
      thread_pool_(
          1,
          vraft::ServerThreadParam{
              "vectordb", 1, false, config_->addr().host, config_->addr().port,
              vraft::TcpOptions(),
              std::bind(&VectorDB::OnMessage, this, std::placeholders::_1,
                        std::placeholders::_2),
              std::bind(&VectorDB::OnConnection, this, std::placeholders::_1),
              nullptr}) {
  path_ = config_->path();
  vengine_ = std::make_shared<AnnoyDB>(path_);
  assert(vengine_);
}

void VectorDB::OnConnection(const vraft::TcpConnectionSPtr &conn) {}

void VectorDB::OnMessage(const vraft::TcpConnectionSPtr &conn,
                         vraft::Buffer *buf) {}

}  // namespace vectordb
