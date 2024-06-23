#ifndef VECTORDB_VINDEX_H_
#define VECTORDB_VINDEX_H_

#include <memory>

namespace vectordb {

class Vindex;
using VindexSPtr = std::shared_ptr<Vindex>;
using VindexUPtr = std::unique_ptr<Vindex>;
using VindexWPtr = std::weak_ptr<Vindex>;

class Vindex final {
 public:
  explicit Vindex();
  ~Vindex();
  Vindex(const Vindex &) = delete;
  Vindex &operator=(const Vindex &) = delete;

 private:
};

inline Vindex::Vindex() {}

inline Vindex::~Vindex() {}

}  // namespace vectordb

#endif
