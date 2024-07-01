#ifndef VSTORE_VSTORE_H_
#define VSTORE_VSTORE_H_

#include <memory>

#include "vstore_common.h"

namespace vstore {

class Vstore final {
 public:
  explicit Vstore();
  ~Vstore();
  Vstore(const Vstore &) = delete;
  Vstore &operator=(const Vstore &) = delete;

  void Start() {}
  void Stop() {}
  void Join() {}

 private:
};

inline Vstore::Vstore() {}

inline Vstore::~Vstore() {}

}  // namespace vstore

#endif
