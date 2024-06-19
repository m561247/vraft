#ifndef VECTORDB_ANNOY_DB_H_
#define VECTORDB_ANNOY_DB_H_

#include "vectordb.h"

namespace vectordb {

class AnnoyDB : public VectorDB {
 public:
  explicit AnnoyDB();
  ~AnnoyDB();
  AnnoyDB(const AnnoyDB &t) = delete;
  AnnoyDB &operator=(const AnnoyDB &t) = delete;

 private:
};

inline AnnoyDB::AnnoyDB() {}

inline AnnoyDB::~AnnoyDB() {}

}  // namespace vectordb

#endif
