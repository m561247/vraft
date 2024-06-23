#include "vectordb.h"

#include <functional>

#include "message.h"
#include "msg_version.h"
#include "msg_version_reply.h"
#include "server_thread.h"
#include "vdb_message.h"
#include "vengine.h"
#include "vraft_logger.h"

namespace vectordb {

VectorDB::VectorDB(VdbConfigSPtr config)
    : start_(false), seqid_(0), config_(config) {
  path_ = config_->path();
  vengine_ = std::make_shared<VEngine>(path_, 3);
  assert(vengine_);

  vraft::ServerThreadParam param;
  param.name = "vectordb";
  param.server_num = 1;
  param.detach = false;
  param.host = config_->addr().host;
  param.start_port = config_->addr().port;
  param.options = vraft::TcpOptions();
  param.on_message_cb = std::bind(&VectorDB::OnMessage, this,
                                  std::placeholders::_1, std::placeholders::_2);
  param.on_connection_cb =
      std::bind(&VectorDB::OnConnection, this, std::placeholders::_1);
  param.write_complete_cb = nullptr;
  server_thread_ = std::make_shared<vraft::ServerThread>(param);
}

void VectorDB::Start() {
  server_thread_->Start();
  server_thread_->WaitStarted();
  start_.store(true);
}
void VectorDB::Join() { server_thread_->Join(); }

void VectorDB::Stop() { server_thread_->Stop(); }

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
          msg.seqid = seqid_.fetch_add(1);
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
