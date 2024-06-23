#include "vdb_console.h"

#include "msg_version.h"
#include "msg_version_reply.h"
#include "util.h"
#include "vdb_message.h"

namespace vectordb {

int32_t VdbConsole::Parse(const std::string &cmd_line) {
  int argc;
  char **argv;
  vraft::ConvertStringToArgcArgv(cmd_line, &argc, &argv);

  std::string cmd = argv[0];
  vraft::ToLower(cmd);
  if (cmd == "version") {
    MsgVersion msg;
    msg.src = 0;
    msg.dest = 0;
    std::string msg_str;
    int32_t bytes = msg.ToString(msg_str);

    vraft::MsgHeader header;
    header.body_bytes = bytes;
    header.type = kVersion;
    std::string header_str;
    header.ToString(header_str);

    header_str.append(std::move(msg_str));
    Send(header_str);
  }

  vraft::FreeArgv(argc, argv);
  return 0;
}

int32_t VdbConsole::Execute() { return 0; }

void VdbConsole::OnMessage(const vraft::TcpConnectionSPtr &conn,
                           vraft::Buffer *buf) {
  if (buf->ReadableBytes() > static_cast<int32_t>(sizeof(vraft::MsgHeader))) {
    int32_t body_bytes = buf->PeekInt32();
    if (buf->ReadableBytes() >=
        static_cast<int32_t>(sizeof(vraft::MsgHeader)) + body_bytes) {
      // parse header
      vraft::MsgHeader header;
      header.FromString(buf->BeginRead(), sizeof(vraft::MsgHeader));
      buf->Retrieve(sizeof(vraft::MsgHeader));

      // parse body
      switch (header.type) {
        case kVersionReply: {
          MsgVersionReply msg;
          int32_t sz = msg.FromString(buf->BeginRead(), body_bytes);
          assert(sz > 0);
          buf->Retrieve(body_bytes);
          set_result(std::string(msg.version));
          break;
        }

        default:
          assert(0);
      }
    }
  }

  ResultReady();
}

}  // namespace vectordb
