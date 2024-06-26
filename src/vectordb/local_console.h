#ifndef VECTORDB_LOCAL_CONSOLE_H_
#define VECTORDB_LOCAL_CONSOLE_H_

#include <memory>

namespace vectordb {

class LocalConsole;
using LocalConsoleSPtr = std::shared_ptr<LocalConsole>;
using LocalConsoleUPtr = std::unique_ptr<LocalConsole>;
using LocalConsoleWPtr = std::weak_ptr<LocalConsole>;

class LocalConsole final {
public:
  explicit LocalConsole();
  ~LocalConsole();
  LocalConsole(const LocalConsole &) = delete;
  LocalConsole &operator=(const LocalConsole &) = delete;

private:
};

inline LocalConsole::LocalConsole() {}

inline LocalConsole::~LocalConsole() {}

} // namespace vectordb

#endif
