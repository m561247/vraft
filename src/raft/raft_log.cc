#include "raft_log.h"

#include <cassert>

#include "coding.h"
#include "leveldb/write_batch.h"

namespace vraft {

// log_index: 1 2 3
// term_key : 1 3 5
// value_key: 2 4 6
RaftIndex LogIndexToTermIndex(RaftIndex log_index) { return log_index * 2 - 1; }

RaftIndex TermIndexToLogIndex(RaftIndex term_index) {
  assert(term_index % 2 == 1);
  return (term_index + 1) / 2;
}

RaftIndex LogIndexToValueIndex(RaftIndex log_index) { return log_index * 2; }

RaftIndex ValueIndexToLogIndex(RaftIndex value_index) {
  assert(value_index % 2 == 0);
  return value_index / 2;
}

void EncodeValueKey(RaftIndex log_index, char *dst) {
  RaftIndex value_key = LogIndexToValueIndex(log_index);
  EncodeFixed32(dst, value_key);
}

void EncodeTermKey(RaftIndex log_index, char *dst) {
  RaftIndex term_index = LogIndexToTermIndex(log_index);
  EncodeFixed32(dst, term_index);
}

void EncodeTermValue(RaftTerm term, char *dst) { EncodeFixed64(dst, term); }

const leveldb::Comparator *U32Comparator() {
  static NoDestructor<U32ComparatorImpl> singleton;
  return singleton.get();
}

U32ComparatorImpl::~U32ComparatorImpl() {}

int U32ComparatorImpl::Compare(const leveldb::Slice &a,
                               const leveldb::Slice &b) const {
  assert(a.size() == sizeof(uint32_t));
  assert(b.size() == sizeof(uint32_t));

  uint32_t da = DecodeFixed32(a.data());
  uint32_t db = DecodeFixed32(b.data());
  int32_t diff = da - db;
  if (diff < 0) {
    return -1;
  } else if (diff > 0) {
    return 1;
  } else {
    return 0;
  }
}

void U32ComparatorImpl::FindShortestSeparator(
    std::string *start, const leveldb::Slice &limit) const {
  // do nothing, just make no warning
  if (start->size() == 0 || limit.size() == 0) return;
}

void U32ComparatorImpl::FindShortSuccessor(std::string *key) const {
  // do nothing, just make no warning
  if (key->size() == 0) return;
}

// index    : 1 2 3
// term_key : 1 3 5
// value_key: 2 4 6
void RaftLog::Init() {
  db_options_.create_if_missing = true;
  db_options_.error_if_exists = false;
  db_options_.comparator = U32Comparator();
  leveldb::DB *dbptr;
  leveldb::Status status = leveldb::DB::Open(db_options_, path_, &dbptr);
  assert(status.ok());
  db_.reset(dbptr);

  leveldb::ReadOptions ro;
  leveldb::Iterator *it = db_->NewIterator(ro);
  it->SeekToFirst();
  if (it->Valid()) {
    std::string begin_key_str = it->key().ToString();
    assert(begin_key_str.size() == sizeof(RaftIndex));
    RaftIndex term_index = DecodeFixed32(begin_key_str.c_str());
    begin_index_ = TermIndexToLogIndex(term_index);

    it->SeekToLast();
    assert(it->Valid());
    std::string end_key_str = it->key().ToString();
    assert(end_key_str.size() == sizeof(RaftIndex));
    RaftIndex value_index = DecodeFixed32(end_key_str.c_str());
    end_index_ = ValueIndexToLogIndex(value_index);

  } else {
    begin_index_ = 0;
    end_index_ = 0;
  }
  delete it;
}

RaftLog::RaftLog(const std::string &path) : path_(path) { Init(); }

int32_t RaftLog::Get(RaftIndex index, LogEntry &entry) { return 0; }

int32_t RaftLog::Append(AppendEntry &entry) {
  if (begin_index_ == 0 && end_index_ == 0) {
    begin_index_ = 1;
    end_index_ = 1;
  } else {
    assert(begin_index_ > 0);
    assert(end_index_ > 0);
    assert(end_index_ >= begin_index_);
    ++end_index_;
  }

  leveldb::WriteBatch batch;
  RaftIndex log_index = end_index_;

  char term_key[sizeof(RaftIndex)];
  EncodeTermKey(log_index, term_key);
  char term_value[sizeof(RaftTerm)];
  EncodeTermValue(entry.term, term_value);
  batch.Put(leveldb::Slice(term_key, sizeof(term_key)),
            leveldb::Slice(term_value, sizeof(term_value)));

  char value_key[sizeof(RaftIndex)];
  EncodeValueKey(log_index, value_key);
  batch.Put(leveldb::Slice(value_key, sizeof(value_key)),
            leveldb::Slice(entry.value.c_str(), entry.value.size()));

  leveldb::WriteOptions wo;
  wo.sync = true;
  leveldb::Status s = db_->Write(wo, &batch);
  assert(s.ok());
  return 0;
}

int32_t RaftLog::DeleteFrom(RaftIndex from_index) {
  if (begin_index_ == 0 && end_index_ == 0) {
    return 0;
  }
  assert(begin_index_ > 0);
  assert(end_index_ > 0);
  assert(end_index_ >= begin_index_);

  if (from_index > end_index_) {
    return 0;
  }

  if (from_index < begin_index_) {
    from_index = begin_index_;
  }
  assert(begin_index_ <= from_index);
  assert(from_index <= end_index_);

  leveldb::WriteBatch batch;
  for (RaftIndex i = from_index; i <= end_index_; ++i) {
    char term_key[sizeof(RaftIndex)];
    EncodeTermKey(i, term_key);
    batch.Delete(leveldb::Slice(term_key, sizeof(term_key)));

    char value_key[sizeof(RaftIndex)];
    EncodeValueKey(i, value_key);
    batch.Delete(leveldb::Slice(value_key, sizeof(value_key)));
  }

  leveldb::Status s = db_->Write(leveldb::WriteOptions(), &batch);
  assert(s.ok());

  end_index_ = from_index - 1;
  if (end_index_ == 0) {
    begin_index_ = 0;
  }

  return 0;
}

int32_t RaftLog::DeleteUtil(RaftIndex to_index) {
  if (begin_index_ == 0 && end_index_ == 0) {
    return 0;
  }
  assert(begin_index_ > 0);
  assert(end_index_ > 0);
  assert(end_index_ >= begin_index_);

  if (to_index < begin_index_) {
    return 0;
  }

  if (to_index > end_index_) {
    to_index = end_index_;
  }
  assert(begin_index_ <= to_index);
  assert(to_index <= end_index_);

  leveldb::WriteBatch batch;
  for (RaftIndex i = begin_index_; i <= to_index; ++i) {
    char term_key[sizeof(RaftIndex)];
    EncodeTermKey(i, term_key);
    batch.Delete(leveldb::Slice(term_key, sizeof(term_key)));

    char value_key[sizeof(RaftIndex)];
    EncodeValueKey(i, value_key);
    batch.Delete(leveldb::Slice(value_key, sizeof(value_key)));
  }

  leveldb::Status s = db_->Write(leveldb::WriteOptions(), &batch);
  assert(s.ok());

  begin_index_ = to_index + 1;
  if (begin_index_ > end_index_) {
    begin_index_ = 0;
    end_index_ = 0;
  }

  return 0;
}

}  // namespace vraft
