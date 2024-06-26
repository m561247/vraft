#ifndef VECTORDB_LOCAL_CONSOLE_H_
#define VECTORDB_LOCAL_CONSOLE_H_

#include <memory>

#include "console.h"
#include "cxxopts.hpp"
#include "vdb_common.h"

namespace vectordb {

class LocalConsole;
using LocalConsoleSPtr = std::shared_ptr<LocalConsole>;
using LocalConsoleUPtr = std::unique_ptr<LocalConsole>;
using LocalConsoleWPtr = std::weak_ptr<LocalConsole>;

class LocalConsole final : public vraft::Console {
 public:
  explicit LocalConsole(const std::string &name, const std::string &path);
  ~LocalConsole();
  LocalConsole(const LocalConsole &) = delete;
  LocalConsole &operator=(const LocalConsole &) = delete;

 private:
  int32_t Parse(const std::string &cmd_line) override;
  int32_t Execute() override;
  void OnMessage(const vraft::TcpConnectionSPtr &conn,
                 vraft::Buffer *buf) override {}

 private:
  void Clear();
  void Help();
  void Error();
  void Quit();
  void Version();

 private:
  // parse result
  int argc_;
  char **argv_;
  std::string cmd_;
  std::shared_ptr<cxxopts::Options> options_;
  cxxopts::ParseResult parse_result_;

  VdbEngineSPtr vdb;
};

inline LocalConsole::~LocalConsole() {}

}  // namespace vectordb

#endif
