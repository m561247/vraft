#include "console.h"

#include <iostream>

#include "client_thread.h"

namespace vraft {

Console::Console(const std::string &name)
    : name_(name), prompt_("(" + name + ")> "), wait_result_(1) {}

Console::Console(const std::string &name, const HostPort &dest)
    : name_(name), prompt_("(" + name + ")> "), wait_result_(1) {
  client_thread_ = std::make_shared<ClientThread>(name_, false);
}

int32_t Console::Run() {
  while (true) {
    // reset
    Reset();

    // get command
    std::cout << prompt_;
    if (!std::getline(std::cin, cmd_line_)) {
      // EOF or error input
      break;
    }

    if (cmd_line_ == "") {
      continue;
    }

    // parse command
    int32_t rv = Parse(cmd_line_);
    if (rv != 0) {
      continue;
    }

    // execute command and wait for result
    rv = Execute();
    WaitResult();

    // show result
    std::cout << result_ << std::endl;
  }

  return 0;
}

void Console::Stop() {}

void Console::Reset() {
  result_.clear();
  cmd_line_.clear();
  wait_result_.Reset();
}

void Console::set_result(const std::string &result) { result_ = result; }

void Console::WaitResult() { wait_result_.Wait(); }

void Console::ResultReady() { wait_result_.CountDown(); }

}  // namespace vraft
