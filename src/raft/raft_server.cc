#include "raft_server.h"

#include "ping.h"
#include "ping_reply.h"
#include "vraft_logger.h"

namespace vraft {

void RaftServer::OnConnection(const vraft::TcpConnectionPtr &conn) {
  vraft::vraft_logger.FInfo("raft-server OnConnection:%s",
                            conn->name().c_str());
}

void RaftServer::OnMessage(const vraft::TcpConnectionPtr &conn,
                           vraft::Buffer *buf) {
  vraft_logger.FTrace("raft-server recv msg, readable-bytes:%d",
                      buf->ReadableBytes());
  int32_t print_bytes = buf->ReadableBytes() > 100 ? 100 : buf->ReadableBytes();
  vraft_logger.FDebug("recv buf data:%s",
                      StrToHexStr(buf->BeginRead(), print_bytes).c_str());

  if (buf->ReadableBytes() > sizeof(sizeof(MsgHeader))) {
    int32_t body_bytes = buf->PeekInt32();
    vraft_logger.FTrace(
        "raft-server recv msg, readable-bytes:%d, body_bytes:%d",
        buf->ReadableBytes(), body_bytes);

    if (buf->ReadableBytes() > sizeof(MsgHeader) + body_bytes) {
      // parse header
      MsgHeader header;
      header.FromString(buf->BeginRead(), sizeof(MsgHeader));
      buf->Retrieve(sizeof(MsgHeader));

      // parse body
      switch (header.type) {
        case kMsgPing: {
          Ping msg;
          bool b = msg.FromString(buf->BeginRead(), body_bytes);
          assert(b);
          buf->Retrieve(body_bytes);
          raft_.OnPing(msg);
          break;
        }
        case kMsgPingReply: {
          PingReply msg;
          bool b = msg.FromString(buf->BeginRead(), body_bytes);
          assert(b);
          buf->Retrieve(body_bytes);
          raft_.OnPingReply(msg);
          break;
        }

        case kRequestVote: {
          break;
        }

        case kRequestVoteReply: {
          break;
        }

        case kAppendEntries: {
          break;
        }

        case kAppendEntriesReply: {
          break;
        }

        default:
          assert(0);
      }
    }
  }
}

int32_t RaftServer::Start() {
  int32_t rv = 0;
  rv = server_->Start();
  assert(rv == 0);

#if 0
  for (auto pair : clients_) {
    rv = pair.second->TimerConnect(INT64_MAX);
    assert(rv == 0);
  }
#endif

  rv = raft_.Start();
  assert(rv == 0);

  rv = loop_.Loop();
  return rv;
}

int32_t RaftServer::Stop() {
  int32_t rv = raft_.Stop();
  assert(rv == 0);
  return rv;
}

}  // namespace vraft
