#include "local_console.h"

#include "vdb_engine.h"
#include "version.h"

namespace vectordb {

LocalConsole::LocalConsole(const std::string &name, const std::string &path)
    : Console(name), argc_(0), argv_(nullptr), cmd_(""), options_(nullptr) {
  vdb = std::make_shared<VdbEngine>(path);
  assert(vdb);
}

int32_t LocalConsole::Parse(const std::string &cmd_line) {
  Clear();
  vraft::ConvertStringToArgcArgv(cmd_line, &argc_, &argv_);
  options_ = std::make_shared<cxxopts::Options>(std::string(argv_[0]));
  parse_result_ = options_->parse(argc_, argv_);
  cmd_ = argv_[0];
  vraft::ToLower(cmd_);
  return 0;
}

int32_t LocalConsole::Execute() {
  if (cmd_ == "help") {
    Help();
    ResultReady();

  } else if (cmd_ == "version") {
    Version();
    ResultReady();

  } else if (cmd_ == "quit") {
    Quit();
    ResultReady();

  } else if (cmd_ == "show-metas") {
    ResultReady();

  } else if (cmd_ == "show-tables") {
    ResultReady();

  } else if (cmd_ == "show-partitions") {
    ResultReady();

  } else if (cmd_ == "show-replicas") {
    ResultReady();

  } else if (cmd_ == "desc") {
    ResultReady();

  } else if (cmd_ == "build-index") {
    ResultReady();

  } else if (cmd_ == "show-metas") {
    ResultReady();

  } else if (cmd_ == "create-table") {
    ResultReady();

  } else if (cmd_ == "create-table") {
    ResultReady();

  } else {
    Error();
    ResultReady();
  }

  return 0;
}

void LocalConsole::Clear() {
  cmd_.clear();
  options_.reset();
  vraft::FreeArgv(argc_, argv_);
}

void LocalConsole::Help() {
  std::string help = "help\n";
  help.append("quit\n");
  help.append("version");
  set_result(help);
}

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
