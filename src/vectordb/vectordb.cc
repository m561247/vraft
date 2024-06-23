#include "vectordb.h"

#include <functional>

#include "annoy_db.h"
#include "message.h"
#include "msg_version.h"
#include "msg_version_reply.h"
#include "server_thread.h"
#include "vdb_message.h"
#include "vraft_logger.h"

namespace vectordb {

VectorDB::VectorDB(VdbConfigSPtr config)
    : start_(false),
      config_(config),
      thread_pool_(
          1,
          vraft::ServerThreadParam{
              "vectordb", 1, false, config_->addr().host, config_->addr().port,
              vraft::TcpOptions(),
              std::bind(&VectorDB::OnMessage, this, std::placeholders::_1,
                        std::placeholders::_2),
              std::bind(&VectorDB::OnConnection, this, std::placeholders::_1),
              nullptr}),
      send_(nullptr) {
  path_ = config_->path();
  vengine_ = std::make_shared<AnnoyDB>(path_);
  assert(vengine_);
}

void VectorDB::Start() {
  thread_pool_.Start();
  // FIXME: use callback!!
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  start_.store(true);
}
void VectorDB::Join() { thread_pool_.Join(); }

void VectorDB::Stop() { thread_pool_.Stop(); }

void VectorDB::OnConnection(const vraft::TcpConnectionSPtr &conn) {
  vraft::vraft_logger.FInfo("vectordb on-connection, %s",
                            conn->DebugString().c_str());
}

void VectorDB::OnMessage(const vraft::TcpConnectionSPtr &conn,
                         vraft::Buffer *buf) {
  vraft::vraft_logger.FTrace("vectordb recv msg, readable-bytes:%d",
                             buf->ReadableBytes());
  int32_t print_bytes = buf->ReadableBytes() > 100 ? 100 : buf->ReadableBytes();
  vraft::vraft_logger.FDebug(
      "recv buf data:%s",
      vraft::StrToHexStr(buf->BeginRead(), print_bytes).c_str());

  if (buf->ReadableBytes() >= static_cast<int32_t>(sizeof(vraft::MsgHeader))) {
    int32_t body_bytes = buf->PeekInt32();
    vraft::vraft_logger.FTrace(
        "vectordb recv msg, readable-bytes:%d, body_bytes:%d",
        buf->ReadableBytes(), body_bytes);

    if (buf->ReadableBytes() >=
        static_cast<int32_t>(sizeof(vraft::MsgHeader)) + body_bytes) {
      // parse header
      vraft::MsgHeader header;
      header.FromString(buf->BeginRead(), sizeof(vraft::MsgHeader));
      buf->Retrieve(sizeof(vraft::MsgHeader));

      // parse body
      switch (header.type) {
        case kVersion: {
          MsgVersion msg;
          int32_t sz = msg.FromString(buf->BeginRead(), body_bytes);
          assert(sz > 0);
          buf->Retrieve(body_bytes);
          OnMsgVersion(conn, msg);
          break;
        }

        default:
          assert(0);
      }
    }
  }
}

void VectorDB::OnMsgVersion(const vraft::TcpConnectionSPtr &conn,
                            struct MsgVersion &msg) {
  if (start_.load()) {
    MsgVersionReply reply;
    reply.src = msg.dest;
    reply.dest = msg.src;
    reply.version = VECTORDB_VERSION;
    std::string reply_str;
    int32_t bytes = reply.ToString(reply_str);

    vraft::MsgHeader header;
    header.body_bytes = bytes;
    header.type = kVersionReply;
    std::string header_str;
    header.ToString(header_str);

    header_str.append(std::move(reply_str));
    conn->CopySend(header_str.data(), header_str.size());
  }
}

}  // namespace vectordb
