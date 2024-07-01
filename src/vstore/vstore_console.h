#ifndef VSTORE_VSTORE_CONSOLE_H_
#define VSTORE_VSTORE_CONSOLE_H_

#include <memory>

namespace vstore {

class VstoreConsole;
using VstoreConsoleSPtr = std::shared_ptr<VstoreConsole>;
using VstoreConsoleUPtr = std::unique_ptr<VstoreConsole>;
using VstoreConsoleWPtr = std::weak_ptr<VstoreConsole>;

class VstoreConsole final {
 public:
  explicit VstoreConsole();
  ~VstoreConsole();
  VstoreConsole(const VstoreConsole &) = delete;
  VstoreConsole &operator=(const VstoreConsole &) = delete;

 private:
};

inline VstoreConsole::VstoreConsole() {}

inline VstoreConsole::~VstoreConsole() {}

}  // namespace vstore

#endif
