#ifndef VECTORDB_VECTORDB_H_
#define VECTORDB_VECTORDB_H_

namespace vectordb {

class VectorDB {
 public:
  explicit VectorDB();
  ~VectorDB();
  VectorDB(const VectorDB &t) = delete;
  VectorDB &operator=(const VectorDB &t) = delete;

 private:
};

inline VectorDB::VectorDB() {}

inline VectorDB::~VectorDB() {}

}  // namespace vectordb

#endif
