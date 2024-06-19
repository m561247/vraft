#ifndef VRAFT_ECHO_CONSOLE_H_
#define VRAFT_ECHO_CONSOLE_H_

#include "console.h"

class EchoConsole : public vraft::Console {
 public:
  EchoConsole(const std::string &name) : Console(name) {}
  ~EchoConsole() {}
  EchoConsole(const EchoConsole &t) = delete;
  EchoConsole &operator=(const EchoConsole &t) = delete;

 private:
  int32_t Parse(const std::string &cmd_line) override { return 0; }
  int32_t Execute() override { return 0; }
};

#endif
