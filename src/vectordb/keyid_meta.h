#ifndef VECTORDB_KEYID_META_H_
#define VECTORDB_KEYID_META_H_

#include <memory>

namespace vectordb {

class KeyidMeta final {
 public:
  explicit KeyidMeta();
  ~KeyidMeta();
  KeyidMeta(const KeyidMeta &) = delete;
  KeyidMeta &operator=(const KeyidMeta &) = delete;

 private:
};

inline KeyidMeta::KeyidMeta() {}

inline KeyidMeta::~KeyidMeta() {}

}  // namespace vectordb

#endif
