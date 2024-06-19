#include "console.h"

#include "client_thread.h"

namespace vraft {

Console::Console(const std::string &name)
    : name_(name), prompt_("(" + name + ")>"), wait_result_(1) {}

Console::Console(const std::string &name, const HostPort &dest)
    : name_(name), prompt_("(" + name + ")>"), wait_result_(1) {
  client_thread_ = std::make_shared<ClientThread>(name_, false);
}

int32_t Console::Run() {
  while (true) {
    std::string cmd_line;
    int32_t rv = Parse(cmd_line);
    if (rv != 0) {
      continue;
    }

    rv = Execute();
    WaitResult();

    // show result
  }

  return 0;
}

void Console::Stop() {}

void Console::WaitResult() { wait_result_.Wait(); }

}  // namespace vraft
