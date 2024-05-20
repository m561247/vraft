#ifndef VRAFT_RAFT_LOG_H_
#define VRAFT_RAFT_LOG_H_

#include <stdint.h>

#include <cassert>
#include <memory>
#include <type_traits>
#include <utility>

#include "allocator.h"
#include "coding.h"
#include "common.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"
#include "nlohmann/json.hpp"
#include "slice.h"
#include "util.h"

namespace vraft {

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
  RaftTerm term;
  std::string value;
};

struct LogEntry {
  RaftIndex index;
  AppendEntry append_entry;

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  int32_t FromString(std::string &s);
  int32_t FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

inline int32_t LogEntry::MaxBytes() {
  return sizeof(RaftIndex) + sizeof(append_entry.term) + 2 * sizeof(int32_t) +
         append_entry.value.size();
}

inline int32_t LogEntry::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr = reinterpret_cast<char *>(DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  DefaultAllocator().Free(ptr);
  return size;
}

inline int32_t LogEntry::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  EncodeFixed32(p, index);
  p += sizeof(index);
  size += sizeof(index);

  EncodeFixed64(p, append_entry.term);
  p += sizeof(append_entry.term);
  size += sizeof(append_entry.term);

  Slice sls(append_entry.value.c_str(), append_entry.value.size());
  char *p2 = EncodeString2(p, len - size, sls);
  size += (p2 - p);

  assert(size <= len);
  return size;
}

inline int32_t LogEntry::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

inline int32_t LogEntry::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  index = DecodeFixed32(p);
  p += sizeof(index);
  size += sizeof(index);

  append_entry.term = DecodeFixed64(p);
  p += sizeof(append_entry.term);
  size += sizeof(append_entry.term);

  Slice result;
  Slice input(p, len - sizeof(index) - sizeof(append_entry.term));
  int32_t sz = DecodeString2(&input, &result);
  if (sz > 0) {
    append_entry.value.clear();
    append_entry.value.append(result.data(), result.size());
    size += sz;

  } else {
    return -1;
  }

  return size;
}

inline nlohmann::json LogEntry::ToJson() {
  nlohmann::json j;
  j["index"] = index;
  j["term"] = append_entry.term;
  j["value"] =
      StrToHexStr(append_entry.value.c_str(), append_entry.value.size());
  j["this"] = PointerToHexStr(this);
  return j;
}

inline nlohmann::json LogEntry::ToJsonTiny() {
  nlohmann::json j;
  j["idx"] = index;
  j["tm"] = append_entry.term;
  j["val"] = StrToHexStr(append_entry.value.c_str(), append_entry.value.size());
  j["ts"] = PointerToHexStr(this);
  return j;
}

inline std::string LogEntry::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["entry"] = ToJsonTiny();
  } else {
    j["log_entry"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

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

class RaftLog;
using RaftLogUPtr = std::unique_ptr<RaftLog>;

const char kAppendKey[sizeof(RaftIndex)] = {0};

class RaftLog final {
 public:
  RaftLog(const std::string &path);
  ~RaftLog();
  RaftLog(const RaftLog &t) = delete;
  RaftLog &operator=(const RaftLog &t) = delete;
  void Init();
  void Check();

  int32_t Get(RaftIndex index, LogEntry &entry);
  int32_t Append(AppendEntry &entry);
  int32_t DeleteFrom(RaftIndex from_index);
  int32_t DeleteUtil(RaftIndex to_index);

  RaftIndex First() const { return first_; };
  RaftIndex Last() const { return last_; }
  RaftIndex Append() const { return append_; }

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);

 private:
  RaftIndex first_;
  RaftIndex last_;
  RaftIndex append_;

  std::string path_;
  leveldb::Options db_options_;
  std::shared_ptr<leveldb::DB> db_;
};

inline RaftLog::~RaftLog() {}

inline nlohmann::json RaftLog::ToJson() {
  nlohmann::json j;
  j["first"] = first_;
  j["last"] = last_;
  return j;
}

inline nlohmann::json RaftLog::ToJsonTiny() {
  nlohmann::json j;
  j[0] = first_;
  j[1] = last_;
  return j;
}

inline std::string RaftLog::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["raft_log"] = ToJsonTiny();
  } else {
    j["rlog"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

}  // namespace vraft

#endif
