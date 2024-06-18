#include "console.h"

namespace vraft {

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

int32_t Console::Parse(const std::string &cmd_line) { return 0; }

int32_t Console::Execute() { return 0; }

void Console::WaitResult() { wait_result_.Wait(); }

}  // namespace vraft
