#ifndef VRAFT_RAFT_LOG_H_
#define VRAFT_RAFT_LOG_H_

#include <stdint.h>

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

#include "leveldb/comparator.h"
#include "leveldb/db.h"

namespace vraft {

using RaftIndex = uint32_t;
using RaftTerm = uint64_t;

// log_index: 1 2 3
// term_key : 1 3 5
// value_key: 2 4 6
RaftIndex LogIndexToTermIndex(RaftIndex log_index);
RaftIndex TermIndexToLogIndex(RaftIndex term_index);
RaftIndex LogIndexToValueIndex(RaftIndex log_index);
RaftIndex ValueIndexToLogIndex(RaftIndex value_index);

void EncodeValueKey(RaftIndex log_index, char *dst);
void EncodeTermKey(RaftIndex log_index, char *dst);
void EncodeTermValue(RaftTerm term, char *dst);

const leveldb::Comparator *U32Comparator();

struct AppendEntry {
  std::string value;
  RaftTerm term;
};

struct LogEntry {
  RaftIndex index;
  AppendEntry append_entry;
};

// Wraps an instance whose destructor is never called.
//
// This is intended for use with function-level static variables.
template <typename InstanceType>
class NoDestructor {
 public:
  template <typename... ConstructorArgTypes>
  explicit NoDestructor(ConstructorArgTypes &&... constructor_args) {
    static_assert(sizeof(instance_storage_) >= sizeof(InstanceType),
                  "instance_storage_ is not large enough to hold the instance");
    static_assert(
        alignof(decltype(instance_storage_)) >= alignof(InstanceType),
        "instance_storage_ does not meet the instance's alignment requirement");
    new (&instance_storage_)
        InstanceType(std::forward<ConstructorArgTypes>(constructor_args)...);
  }

  ~NoDestructor() = default;

  NoDestructor(const NoDestructor &) = delete;
  NoDestructor &operator=(const NoDestructor &) = delete;

  InstanceType *get() {
    return reinterpret_cast<InstanceType *>(&instance_storage_);
  }

 private:
  typename std::aligned_storage<sizeof(InstanceType),
                                alignof(InstanceType)>::type instance_storage_;
};

class U32ComparatorImpl : public leveldb::Comparator {
 public:
  U32ComparatorImpl() {}

  U32ComparatorImpl(const U32ComparatorImpl &) = delete;
  U32ComparatorImpl &operator=(const U32ComparatorImpl &) = delete;

  virtual const char *Name() const { return "leveldb.U32ComparatorImpl"; }

  virtual int Compare(const leveldb::Slice &a, const leveldb::Slice &b) const;
  virtual void FindShortestSeparator(std::string *start,
                                     const leveldb::Slice &limit) const;
  virtual void FindShortSuccessor(std::string *key) const;

  ~U32ComparatorImpl();

 private:
};

class RaftLog final {
 public:
  RaftLog(const std::string &path);
  ~RaftLog();
  RaftLog(const RaftLog &t) = delete;
  RaftLog &operator=(const RaftLog &t) = delete;

  int32_t Get(RaftIndex index, LogEntry &entry);
  int32_t Append(AppendEntry &entry);
  int32_t DeleteFrom(RaftIndex from_index);
  int32_t DeleteUtil(RaftIndex to_index);

  RaftIndex begin_index() const { return begin_index_; };
  RaftIndex end_index() const { return end_index_; }

 private:
  void Init();

 private:
  RaftIndex begin_index_;
  RaftIndex end_index_;

  std::string path_;
  leveldb::Options db_options_;
  std::shared_ptr<leveldb::DB> db_;
};

inline RaftLog::~RaftLog() {}

}  // namespace vraft

#endif
