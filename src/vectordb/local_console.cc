#include "local_console.h"

#include "parser.h"
#include "vdb_engine.h"
#include "version.h"

namespace vectordb {

LocalConsole::LocalConsole(const std::string &name, const std::string &path)
    : Console(name) {
  vdb = std::make_shared<VdbEngine>(path);
  assert(vdb);
}

int32_t LocalConsole::Parse(const std::string &cmd_line) {
  Clear();
  parser_ = std::make_shared<Parser>(cmd_line);
  return 0;
}

int32_t LocalConsole::Execute() {
  if (parser_) {
    switch (parser_->cmd()) {
      case kCmdHelp: {
        Help();
        break;
      }

      case kCmdVersion: {
        Version();
        break;
      }

      case kCmdQuit: {
        Quit();
        break;
      }

      case kCmdMeta: {
        Help();
        break;
      }

      case kCmdPut: {
        Help();
        break;
      }

      case kCmdGet: {
        Help();
        break;
      }

      case kCmdDelete: {
        Help();
        break;
      }

      case kCmdGetKNN: {
        Help();
        break;
      }

      case kCmdLoad: {
        Help();
        break;
      }

      case kCmdCreateTable: {
        Help();
        break;
      }

      case kCmdBuildIndex: {
        Help();
        break;
      }

      case kDescTable: {
        Help();
        break;
      }

      case kDescPartition: {
        Help();
        break;
      }

      case kDescDescReplica: {
        Help();
        break;
      }

      case kShowTables: {
        Help();
        break;
      }

      case kShowPartitions: {
        Help();
        break;
      }

      case kShowReplicas: {
        Help();
        break;
      }

      default:
        Error();
        break;
    }
    ResultReady();
  }

  return 0;
}

void LocalConsole::Clear() { parser_.reset(); }

void LocalConsole::Help() { set_result(HelpStr()); }

void LocalConsole::Error() {
  std::string err = "error command";
  set_result(err);
}

void LocalConsole::Quit() {
  Clear();
  Console::Stop();
}

void LocalConsole::Version() { set_result(VECTORDB_VERSION); }

}  // namespace vectordb
