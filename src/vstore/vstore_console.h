#ifndef VSTORE_VSTORE_CONSOLE_H_
#define VSTORE_VSTORE_CONSOLE_H_

#include <memory>

#include "console.h"

namespace vstore {

class VstoreConsole;
using VstoreConsoleSPtr = std::shared_ptr<VstoreConsole>;
using VstoreConsoleUPtr = std::unique_ptr<VstoreConsole>;
using VstoreConsoleWPtr = std::weak_ptr<VstoreConsole>;

class VstoreConsole : public vraft::Console {
 public:
  explicit VstoreConsole(const std::string &name, const std::string &addr);
  ~VstoreConsole();
  VstoreConsole(const VstoreConsole &) = delete;
  VstoreConsole &operator=(const VstoreConsole &) = delete;

 private:
  int32_t Parse(const std::string &cmd_line) override { return 0; }

  int32_t Execute() override {
    std::string msg = cmd_line();
    Send(msg);
    return 0;
  }

  void OnMessage(const vraft::TcpConnectionSPtr &conn,
                 vraft::Buffer *buf) override {
    set_result("msg back: " + std::string(buf->Peek()));
    buf->RetrieveAll();
    ResultReady();
  }

 private:
};

inline VstoreConsole::VstoreConsole(const std::string &name,
                                    const std::string &addr)
    : Console(name, vraft::HostPort(addr)) {}

inline VstoreConsole::~VstoreConsole() {}

}  // namespace vstore

#endif
