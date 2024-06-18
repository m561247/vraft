#ifndef VRAFT_CONSOLE_H_
#define VRAFT_CONSOLE_H_

#include <string>

#include "count_down.h"

namespace vraft {

class Console final {
 public:
  explicit Console();
  ~Console();
  Console(const Console &t) = delete;
  Console &operator=(const Console &t) = delete;

  int32_t Run();

 private:
  int32_t Parse(const std::string &cmd_line);
  int32_t Execute();
  void WaitResult();

 private:
  std::string result_;
  CountDownLatch wait_result_;
};

inline Console::Console() : wait_result_(1) {}

inline Console::~Console() {}

}  // namespace vraft

#endif
