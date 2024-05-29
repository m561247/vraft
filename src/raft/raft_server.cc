#include "raft_server.h"

#include "ping.h"
#include "ping_reply.h"
#include "vraft_logger.h"

namespace vraft {

void Remu::Print(bool tiny, bool one_line) {
  printf("---remu--- :\n");
  for (auto ptr : raft_servers) {
    ptr->Print(tiny, one_line);
    if (!one_line) {
      printf("\n");
    }
  }
  printf("\n");
  fflush(nullptr);
}

void Remu::Create() {
  for (auto conf : configs) {
    vraft::RaftServerPtr ptr = std::make_shared<vraft::RaftServer>(conf, loop);
    raft_servers.push_back(ptr);
  }
}

void Remu::Start() {
  for (auto &ptr : raft_servers) {
    if (ptr) {
      ptr->Start();
    }
  }
}

void Remu::Stop() {
  for (auto &ptr : raft_servers) {
    if (ptr) {
      ptr->Stop();
    }
  }
}

void Remu::Clear() { raft_servers.clear(); }

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

  if (buf->ReadableBytes() > static_cast<int32_t>(sizeof(MsgHeader))) {
    int32_t body_bytes = buf->PeekInt32();
    vraft_logger.FTrace(
        "raft-server recv msg, readable-bytes:%d, body_bytes:%d",
        buf->ReadableBytes(), body_bytes);

    if (buf->ReadableBytes() >
        static_cast<int32_t>(sizeof(MsgHeader)) + body_bytes) {
      // parse header
      MsgHeader header;
      header.FromString(buf->BeginRead(), sizeof(MsgHeader));
      buf->Retrieve(sizeof(MsgHeader));

      // parse body
      switch (header.type) {
        case kPing: {
          Ping msg;
          bool b = msg.FromString(buf->BeginRead(), body_bytes);
          assert(b);
          buf->Retrieve(body_bytes);
          raft_->OnPing(msg);
          break;
        }
        case kPingReply: {
          PingReply msg;
          bool b = msg.FromString(buf->BeginRead(), body_bytes);
          assert(b);
          buf->Retrieve(body_bytes);
          raft_->OnPingReply(msg);
          break;
        }

        case kRequestVote: {
          RequestVote msg;
          bool b = msg.FromString(buf->BeginRead(), body_bytes);
          assert(b);
          buf->Retrieve(body_bytes);
          raft_->OnRequestVote(msg);
          break;
        }

        case kRequestVoteReply: {
          RequestVoteReply msg;
          bool b = msg.FromString(buf->BeginRead(), body_bytes);
          assert(b);
          buf->Retrieve(body_bytes);
          raft_->OnRequestVoteReply(msg);
          break;
        }

        case kAppendEntries: {
          AppendEntries msg;
          int32_t bytes = msg.FromString(buf->BeginRead(), body_bytes);
          assert(bytes > 0);
          buf->Retrieve(body_bytes);
          raft_->OnAppendEntries(msg);
          break;
        }

        case kAppendEntriesReply: {
          AppendEntriesReply msg;
          bool b = msg.FromString(buf->BeginRead(), body_bytes);
          assert(b);
          buf->Retrieve(body_bytes);
          raft_->OnAppendEntriesReply(msg);
          break;
        }

        case kInstallSnapshot: {
          InstallSnapshot msg;
          int32_t bytes = msg.FromString(buf->BeginRead(), body_bytes);
          assert(bytes > 0);
          buf->Retrieve(body_bytes);
          raft_->OnInstallSnapshot(msg);
          break;
        }

        case kInstallSnapshotReply: {
          InstallSnapshotReply msg;
          bool b = msg.FromString(buf->BeginRead(), body_bytes);
          assert(b);
          buf->Retrieve(body_bytes);
          raft_->OnInstallSnapshotReply(msg);
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

  rv = raft_->Start();
  assert(rv == 0);

  return rv;
}

int32_t RaftServer::Stop() {
  int32_t rv = raft_->Stop();
  assert(rv == 0);
  return rv;
}

int32_t RaftServer::Send(uint64_t dest_addr, const char *buf,
                         unsigned int size) {
  int32_t rv = 0;
  TcpClientPtr client = GetClientOrCreate(dest_addr);
  if (client) {
    rv = client->CopySend(buf, size);
  } else {
    rv = -1;
    vraft_logger.FError("GetClientOrCreate error");
  }
  return rv;
}

TimerPtr RaftServer::MakeTimer(TimerParam &param) {
  return loop_->MakeTimer(param);
}

}  // namespace vraft
