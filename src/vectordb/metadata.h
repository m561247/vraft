#ifndef VECTORDB_METADATA_H_
#define VECTORDB_METADATA_H_

namespace vectordb {

class Metadata final {
 public:
  explicit Metadata();
  ~Metadata();
  Metadata(const Metadata &) = delete;
  Metadata &operator=(const Metadata &) = delete;

 private:
};

inline Metadata::Metadata() {}

inline Metadata::~Metadata() {}

}  // namespace vectordb

#endif
