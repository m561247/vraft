#include "solid_data.h"
#include "coding.h"

namespace vraft {

SolidData::SolidData(const std::string &path) : path_(path) {
  db_options_.create_if_missing = true;
  db_options_.error_if_exists = false;
  leveldb::DB *dbptr;
  leveldb::Status status = leveldb::DB::Open(db_options_, path_, &dbptr);
  assert(status.ok());
  db_.reset(dbptr);

  {
    std::string value;
    leveldb::Slice sls_key(reinterpret_cast<const char *>(&kTermKey),
                           sizeof(kTermKey));
    status = db_->Get(leveldb::ReadOptions(), sls_key, &value);
    if (status.IsNotFound()) {
      term_ = 0;
      char value_buf[sizeof(term_)];
      EncodeFixed64(value_buf, term_);
      leveldb::Slice sls_value(value_buf, sizeof(term_));
      status = db_->Put(leveldb::WriteOptions(), sls_key, sls_value);
      assert(status.ok());
    } else {
      assert(value.size() == sizeof(term_));
      term_ = DecodeFixed64(value.c_str());
    }
  }

  {
    std::string value;
    leveldb::Slice sls_key(reinterpret_cast<const char *>(&kVoteKey),
                           sizeof(kVoteKey));
    status = db_->Get(leveldb::ReadOptions(), sls_key, &value);
    if (status.IsNotFound()) {
      vote_ = 0;
      char value_buf[sizeof(vote_)];
      EncodeFixed64(value_buf, vote_);
      leveldb::Slice sls_value(value_buf, sizeof(vote_));
      status = db_->Put(leveldb::WriteOptions(), sls_key, sls_value);
      assert(status.ok());
    } else {
      assert(value.size() == sizeof(vote_));
      vote_ = DecodeFixed64(value.c_str());
    }
  }
}

void SolidData::IncrTerm() {
  ++term_;
  PersistTerm();
}

void SolidData::SetTerm(uint64_t term) {
  term_ = term;
  PersistTerm();
}

void SolidData::SetVote(uint64_t vote) {
  vote_ = vote;
  PersistVote();
}

void SolidData::PersistTerm() {
  leveldb::Slice sls_key(reinterpret_cast<const char *>(&kTermKey),
                         sizeof(kTermKey));
  char value_buf[sizeof(term_)];
  EncodeFixed64(value_buf, term_);
  leveldb::Slice sls_value(value_buf, sizeof(term_));
  leveldb::Status status =
      db_->Put(leveldb::WriteOptions(), sls_key, sls_value);
  assert(status.ok());
}

void SolidData::PersistVote() {
  leveldb::Slice sls_key(reinterpret_cast<const char *>(&kVoteKey),
                         sizeof(kVoteKey));
  char value_buf[sizeof(vote_)];
  EncodeFixed64(value_buf, vote_);
  leveldb::Slice sls_value(value_buf, sizeof(vote_));
  leveldb::Status status =
      db_->Put(leveldb::WriteOptions(), sls_key, sls_value);
  assert(status.ok());
}

} // namespace vraft
