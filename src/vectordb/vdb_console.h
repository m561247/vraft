#ifndef VECTORDB_VDB_CONSOLE_H_
#define VECTORDB_VDB_CONSOLE_H_

#include <memory>

#include "console.h"

namespace vectordb {

class VdbConsole;
using VdbConsoleSPtr = std::shared_ptr<VdbConsole>;
using VdbConsoleUPtr = std::unique_ptr<VdbConsole>;
using VdbConsoleWPtr = std::weak_ptr<VdbConsole>;

class VdbConsole : public vraft::Console {
 public:
  VdbConsole(const std::string &name, const std::string &addr)
      : Console(name, vraft::HostPort(addr)) {}
  ~VdbConsole() {}
  VdbConsole(const VdbConsole &t) = delete;
  VdbConsole &operator=(const VdbConsole &t) = delete;

 private:
  int32_t Parse(const std::string &cmd_line) override;
  int32_t Execute() override;
  void OnMessage(const vraft::TcpConnectionSPtr &conn,
                 vraft::Buffer *buf) override;
};

}  // namespace vectordb

#endif
