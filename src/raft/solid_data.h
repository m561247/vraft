#ifndef VRAFT_SOLID_DATA_H_
#define VRAFT_SOLID_DATA_H_

#include "leveldb/db.h"
#include <cassert>
#include <memory>
#include <stdint.h>

namespace vraft {

const uint8_t kTermKey = 0;
const uint8_t kVoteKey = 1;

class SolidData final {
public:
  SolidData(const std::string &path);
  ~SolidData();
  SolidData(const SolidData &t) = delete;
  SolidData &operator=(const SolidData &t) = delete;

  std::string path() const { return path_; }

  uint64_t term() const { return term_; }
  uint64_t vote() const { return vote_; }

  void IncrTerm();
  void SetTerm(uint64_t term);
  void SetVote(uint64_t vote);

private:
  void PersistTerm();
  void PersistVote();

private:
  uint64_t term_;
  uint64_t vote_;

  std::string path_;
  leveldb::Options db_options_;
  std::shared_ptr<leveldb::DB> db_;
};

inline SolidData::~SolidData() {}

} // namespace vraft

#endif
