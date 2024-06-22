#ifndef VECTORDB_ANNOY_DB_H_
#define VECTORDB_ANNOY_DB_H_

#include "vengine.h"

namespace vectordb {

class AnnoyDB : public VEngine {
 public:
  explicit AnnoyDB(const std::string &path);
  ~AnnoyDB();
  AnnoyDB(const AnnoyDB &t) = delete;
  AnnoyDB &operator=(const AnnoyDB &t) = delete;

 private:
};

inline AnnoyDB::AnnoyDB(const std::string &path) : VEngine(path) {}

inline AnnoyDB::~AnnoyDB() {}

}  // namespace vectordb

#endif
