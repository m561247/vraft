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
  std::string err_str = status.ToString();
  assert(status.ok());
  db_.reset(dbptr);

  leveldb::ReadOptions ro;
  leveldb::Iterator *it = db_->NewIterator(ro);

  // maybe init first
  it->SeekToFirst();
  if (!it->Valid()) {
    leveldb::WriteBatch batch;
    char value_buf[sizeof(RaftIndex)];
    EncodeFixed32(value_buf, 1);
    batch.Put(leveldb::Slice(kAppendKey, sizeof(kAppendKey)),
              leveldb::Slice(value_buf, sizeof(value_buf)));
    leveldb::WriteOptions wo;
    wo.sync = true;
    leveldb::Status s = db_->Write(wo, &batch);
    assert(s.ok());

    first_ = 0;
    last_ = 0;
    append_ = 1;
  }
  delete it;

  // init
  it = db_->NewIterator(ro);
  it->SeekToLast();
  assert(it->Valid());
  assert(it->key().ToString().size() == sizeof(RaftIndex));
  RaftIndex key_index = DecodeFixed32(it->key().ToString().c_str());

  if (key_index == 0) {
    // no value
    first_ = 0;
    last_ = 0;
    assert(it->value().ToString().size() == sizeof(RaftIndex));
    append_ = DecodeFixed32(it->value().ToString().c_str());

  } else {  // has value
    // last, value index
    std::string key_str = it->key().ToString();
    RaftIndex value_index = DecodeFixed32(key_str.c_str());
    last_ = ValueIndexToLogIndex(value_index);

    // first, term index
    it->SeekToFirst();
    it->Next();
    key_str = it->key().ToString();
    RaftIndex term_index = DecodeFixed32(key_str.c_str());
    first_ = TermIndexToLogIndex(term_index);

    // append
    append_ = last_ + 1;
  }

  delete it;
  Check();
}

void RaftLog::Check() {
  if (first_ == last_) {
    if (first_ == 0) {
      // no value
      assert(append_ >= 1);
    }

    if (first_ > 0) {
      // has value
      assert(append_ == last_ + 1);
    }

  } else {
    // has value
    assert(first_ <= last_);
    assert(first_ >= 1);
    assert(append_ == last_ + 1);
  }
}

RaftLog::RaftLog(const std::string &path) : path_(path) {}

int32_t RaftLog::Get(RaftIndex index, LogEntry &entry) {
  Check();

  char term_key[sizeof(RaftIndex)];
  EncodeTermKey(index, term_key);
  leveldb::ReadOptions ro;
  leveldb::Status s;
  std::string value;
  s = db_->Get(ro, leveldb::Slice(term_key, sizeof(term_key)), &value);
  if (s.ok()) {
    assert(value.size() == sizeof(RaftTerm));
    entry.index = index;
    entry.append_entry.term = DecodeFixed64(value.c_str());

  } else {
    return -1;
  }

  char value_key[sizeof(RaftIndex)];
  EncodeValueKey(index, value_key);
  s = db_->Get(ro, leveldb::Slice(value_key, sizeof(value_key)), &value);
  assert(s.ok());
  entry.append_entry.value = value;

  return 0;
}

LogEntryPtr RaftLog::LastEntry() {
  Check();
  if (last_ == 0) {
    return nullptr;

  } else {
    LogEntryPtr ptr = std::make_shared<LogEntry>();
    int32_t rv = Get(last_, *(ptr.get()));
    assert(rv == 0);
    return ptr;
  }
}

int32_t RaftLog::Append(AppendEntry &entry) {
  Check();
  RaftIndex tmp_first, tmp_last, tmp_append;

  if (first_ == last_ && first_ == 0) {
    // no value
    tmp_first = append_;
    tmp_last = append_;
    tmp_append = append_ + 1;

  } else {
    // has value
    // do not need to persist append index every time

    tmp_first = first_;
    tmp_last = last_ + 1;
    tmp_append = tmp_last + 1;
  }

  leveldb::WriteBatch batch;
  RaftIndex log_index = tmp_last;

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

  // update index after persist value
  // do not need to persist append index every time
  first_ = tmp_first;
  last_ = tmp_last;
  append_ = tmp_append;

  Check();
  return 0;
}

int32_t RaftLog::DeleteFrom(RaftIndex from_index) {
  Check();
  RaftIndex tmp_first, tmp_last, tmp_append;

  // no value
  if (first_ == last_ && first_ == 0) {
    return 0;
  }

  // has value
  assert(first_ > 0);
  assert(last_ > 0);
  assert(last_ >= first_);
  assert(append_ == last_ + 1);

  // adjust from index
  if (from_index > last_) {
    return 0;
  }

  // adjust from index
  if (from_index < first_) {
    from_index = first_;
  }
  assert(first_ <= from_index);
  assert(from_index <= last_);

  // save update index
  if (from_index == first_) {
    tmp_first = 0;
    tmp_last = 0;
    tmp_append = last_ + 1;

  } else {
    assert(from_index > first_);
    tmp_first = first_;
    tmp_last = from_index - 1;
    tmp_append = from_index;
  }

  leveldb::WriteBatch batch;
  for (RaftIndex i = from_index; i <= last_; ++i) {
    char term_key[sizeof(RaftIndex)];
    EncodeTermKey(i, term_key);
    batch.Delete(leveldb::Slice(term_key, sizeof(term_key)));

    char value_key[sizeof(RaftIndex)];
    EncodeValueKey(i, value_key);
    batch.Delete(leveldb::Slice(value_key, sizeof(value_key)));
  }

  // need to persist append index
  if (from_index == first_) {
    char value_buf[sizeof(RaftIndex)];
    EncodeFixed32(value_buf, tmp_append);
    batch.Put(leveldb::Slice(kAppendKey, sizeof(kAppendKey)),
              leveldb::Slice(value_buf, sizeof(value_buf)));
  }

  leveldb::Status s = db_->Write(leveldb::WriteOptions(), &batch);
  assert(s.ok());

  // update index
  first_ = tmp_first;
  last_ = tmp_last;
  append_ = tmp_append;

  Check();
  return 0;
}

int32_t RaftLog::DeleteUtil(RaftIndex to_index) {
  Check();
  RaftIndex tmp_first, tmp_last, tmp_append;

  // no value
  if (first_ == last_ && first_ == 0) {
    return 0;
  }

  // has value
  assert(first_ > 0);
  assert(last_ > 0);
  assert(last_ >= first_);
  assert(append_ == last_ + 1);

  // adjust to index
  if (to_index < first_) {
    return 0;
  }

  // adjust to index
  if (to_index > last_) {
    to_index = last_;
  }
  assert(first_ <= to_index);
  assert(to_index <= last_);

  // save update index
  if (to_index == last_) {
    tmp_first = 0;
    tmp_last = 0;
    tmp_append = last_ + 1;

  } else {
    assert(to_index < last_);
    tmp_first = to_index + 1;
    tmp_last = last_;
    tmp_append = last_ + 1;
  }

  leveldb::WriteBatch batch;
  for (RaftIndex i = first_; i <= to_index; ++i) {
    char term_key[sizeof(RaftIndex)];
    EncodeTermKey(i, term_key);
    batch.Delete(leveldb::Slice(term_key, sizeof(term_key)));

    char value_key[sizeof(RaftIndex)];
    EncodeValueKey(i, value_key);
    batch.Delete(leveldb::Slice(value_key, sizeof(value_key)));
  }

  // need to persist append index
  if (to_index == last_) {
    char value_buf[sizeof(RaftIndex)];
    EncodeFixed32(value_buf, tmp_append);
    batch.Put(leveldb::Slice(kAppendKey, sizeof(kAppendKey)),
              leveldb::Slice(value_buf, sizeof(value_buf)));
  }

  leveldb::Status s = db_->Write(leveldb::WriteOptions(), &batch);
  assert(s.ok());

  // update index
  first_ = tmp_first;
  last_ = tmp_last;
  append_ = tmp_append;

  Check();
  return 0;
}

}  // namespace vraft
