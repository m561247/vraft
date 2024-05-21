#include "raft_log.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>

#include "coding.h"
#include "leveldb/comparator.h"
#include "leveldb/db.h"

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

class I64ComparatorImpl : public leveldb::Comparator {
 public:
  I64ComparatorImpl() {}

  I64ComparatorImpl(const I64ComparatorImpl &) = delete;
  I64ComparatorImpl &operator=(const I64ComparatorImpl &) = delete;

  virtual const char *Name() const { return "leveldb.I64ComparatorImpl"; }

  virtual int Compare(const leveldb::Slice &a, const leveldb::Slice &b) const {
    assert(a.size() == sizeof(int64_t));
    assert(b.size() == sizeof(int64_t));

    int64_t da = vraft::DecodeFixed64(a.data());
    int64_t db = vraft::DecodeFixed64(b.data());
    if (da - db < 0) {
      return -1;
    } else if (da - db > 0) {
      return 1;
    } else {
      return 0;
    }
  }
  virtual void FindShortestSeparator(std::string *start,
                                     const leveldb::Slice &limit) const {
    // do nothing, just make no warning
    if (start->size() == 0 || limit.size() == 0) return;
  }

  virtual void FindShortSuccessor(std::string *key) const {
    // do nothing, just make no warning
    if (key->size() == 0) return;
  }

  ~I64ComparatorImpl() {}

 private:
};

const leveldb::Comparator *I64Comparator() {
  static NoDestructor<I64ComparatorImpl> singleton;
  return singleton.get();
}

//--------------------------------------------------------------------------

class TestU32ComparatorImpl : public leveldb::Comparator {
 public:
  TestU32ComparatorImpl() {}

  TestU32ComparatorImpl(const TestU32ComparatorImpl &) = delete;
  TestU32ComparatorImpl &operator=(const TestU32ComparatorImpl &) = delete;

  virtual const char *Name() const { return "leveldb.TestU32ComparatorImpl"; }

  virtual int Compare(const leveldb::Slice &a, const leveldb::Slice &b) const {
    assert(a.size() == sizeof(uint32_t));
    assert(b.size() == sizeof(uint32_t));

    uint32_t da = vraft::DecodeFixed32(a.data());
    uint32_t db = vraft::DecodeFixed32(b.data());
    int32_t diff = da - db;
    if (diff < 0) {
      return -1;
    } else if (diff > 0) {
      return 1;
    } else {
      return 0;
    }
  }
  virtual void FindShortestSeparator(std::string *start,
                                     const leveldb::Slice &limit) const {
    // do nothing, just make no warning
    if (start->size() == 0 || limit.size() == 0) return;
  }

  virtual void FindShortSuccessor(std::string *key) const {
    // do nothing, just make no warning
    if (key->size() == 0) return;
  }

  ~TestU32ComparatorImpl() {}

 private:
};

const leveldb::Comparator *TestU32Comparator() {
  static NoDestructor<TestU32ComparatorImpl> singleton;
  return singleton.get();
}

//--------------------------------------------------------------------------
class TestI32ComparatorImpl : public leveldb::Comparator {
 public:
  TestI32ComparatorImpl() {}

  TestI32ComparatorImpl(const TestI32ComparatorImpl &) = delete;
  TestI32ComparatorImpl &operator=(const TestI32ComparatorImpl &) = delete;

  virtual const char *Name() const { return "leveldb.TestI32ComparatorImpl"; }

  virtual int Compare(const leveldb::Slice &a, const leveldb::Slice &b) const {
    assert(a.size() == sizeof(int32_t));
    assert(b.size() == sizeof(int32_t));

    int32_t da = vraft::DecodeFixed32(a.data());
    int32_t db = vraft::DecodeFixed32(b.data());
    if (da - db < 0) {
      return -1;
    } else if (da - db > 0) {
      return 1;
    } else {
      return 0;
    }
  }
  virtual void FindShortestSeparator(std::string *start,
                                     const leveldb::Slice &limit) const {
    // do nothing, just make no warning
    if (start->size() == 0 || limit.size() == 0) return;
  }

  virtual void FindShortSuccessor(std::string *key) const {
    // do nothing, just make no warning
    if (key->size() == 0) return;
  }

  ~TestI32ComparatorImpl() {}

 private:
};

const leveldb::Comparator *TestI32Comparator() {
  static NoDestructor<TestI32ComparatorImpl> singleton;
  return singleton.get();
}

//--------------------------------------------------------------------------

TEST(leveldb, test_i64) {
  system("rm -rf /tmp/leveldb_test_dir");
  std::string path = "/tmp/leveldb_test_dir";

  leveldb::Options o;
  o.create_if_missing = true;
  o.error_if_exists = false;
  o.comparator = I64Comparator();
  leveldb::DB *db;
  leveldb::Status status = leveldb::DB::Open(o, path, &db);
  assert(status.ok());

  for (int64_t i = 0; i < 10; ++i) {
    char key_buf[sizeof(int64_t)];
    vraft::EncodeFixed64(key_buf, i);
    leveldb::Slice sls_key(key_buf, sizeof(key_buf));

    char value_buf[128];
    snprintf(value_buf, sizeof(value_buf), "value_%lu", i);
    std::string value_str(value_buf);
    leveldb::Slice sls_value(value_str.c_str(), value_str.size() + 1);

    leveldb::WriteOptions wo;
    leveldb::Status s = db->Put(wo, sls_key, sls_value);
    assert(s.ok());
  }

  leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::string key_str = it->key().ToString();
    assert(key_str.size() == sizeof(int64_t));
    int64_t key64 = vraft::DecodeFixed64(key_str.c_str());

    std::string tmp_key;
    for (auto c : key_str) {
      char buf[2];
      snprintf(buf, sizeof(buf), "%X", c);
      tmp_key.append(buf);
    }

    std::string value_str = it->value().ToString();
    std::cout << tmp_key << " " << key64 << " : " << value_str << std::endl;
  }
  delete it;
  delete db;

  system("rm -rf /tmp/leveldb_test_dir");
}

TEST(leveldb, test_i32) {
  system("rm -rf /tmp/leveldb_test_dir");
  std::string path = "/tmp/leveldb_test_dir";

  leveldb::Options o;
  o.create_if_missing = true;
  o.error_if_exists = false;
  // o.comparator = vraft::U32Comparator();
  o.comparator = TestI32Comparator();
  leveldb::DB *db;
  leveldb::Status status = leveldb::DB::Open(o, path, &db);
  assert(status.ok());

  for (int32_t i = 0; i < 10; ++i) {
    char key_buf[sizeof(int32_t)];
    vraft::EncodeFixed32(key_buf, i);
    leveldb::Slice sls_key(key_buf, sizeof(key_buf));

    char value_buf[128];
    snprintf(value_buf, sizeof(value_buf), "value_%u", i);
    std::string value_str(value_buf);
    leveldb::Slice sls_value(value_str.c_str(), value_str.size() + 1);

    leveldb::WriteOptions wo;
    leveldb::Status s = db->Put(wo, sls_key, sls_value);
    assert(s.ok());
  }

  leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::string key_str = it->key().ToString();
    assert(key_str.size() == sizeof(int32_t));
    int32_t key32 = vraft::DecodeFixed32(key_str.c_str());

    std::string tmp_key;
    for (auto c : key_str) {
      char buf[2];
      snprintf(buf, sizeof(buf), "%X", c);
      tmp_key.append(buf);
    }

    std::string value_str = it->value().ToString();
    std::cout << tmp_key << " " << key32 << " : " << value_str << std::endl;
  }
  delete it;
  delete db;

  system("rm -rf /tmp/leveldb_test_dir");
}

TEST(leveldb, test_u32) {
  system("rm -rf /tmp/leveldb_test_dir");
  std::string path = "/tmp/leveldb_test_dir";

  leveldb::Options o;
  o.create_if_missing = true;
  o.error_if_exists = false;
  // o.comparator = vraft::U32Comparator();
  o.comparator = TestU32Comparator();
  leveldb::DB *db;
  leveldb::Status status = leveldb::DB::Open(o, path, &db);
  assert(status.ok());

  for (uint32_t i = 0; i < 10; ++i) {
    char key_buf[sizeof(uint32_t)];
    vraft::EncodeFixed32(key_buf, i);
    leveldb::Slice sls_key(key_buf, sizeof(key_buf));

    char value_buf[128];
    snprintf(value_buf, sizeof(value_buf), "value_%u", i);
    std::string value_str(value_buf);
    leveldb::Slice sls_value(value_str.c_str(), value_str.size() + 1);

    leveldb::WriteOptions wo;
    leveldb::Status s = db->Put(wo, sls_key, sls_value);
    assert(s.ok());
  }

  leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    std::string key_str = it->key().ToString();
    assert(key_str.size() == sizeof(uint32_t));
    uint32_t key32 = vraft::DecodeFixed32(key_str.c_str());

    std::string tmp_key;
    for (auto c : key_str) {
      char buf[2];
      snprintf(buf, sizeof(buf), "%X", c);
      tmp_key.append(buf);
    }

    std::string value_str = it->value().ToString();
    std::cout << tmp_key << " " << key32 << " : " << value_str << std::endl;
  }
  delete it;
  delete db;

  system("rm -rf /tmp/leveldb_test_dir");
}

// index    : 1 2 3
// term_key : 1 3 5
// value_key: 2 4 6
TEST(coding, test) {
  EXPECT_EQ(vraft::LogIndexToMetaIndex(1), 1);
  EXPECT_EQ(vraft::LogIndexToMetaIndex(3), 5);

  EXPECT_EQ(vraft::LogIndexToDataIndex(1), 2);
  EXPECT_EQ(vraft::LogIndexToDataIndex(3), 6);

  EXPECT_EQ(vraft::MetaIndexToLogIndex(1), 1);
  EXPECT_EQ(vraft::MetaIndexToLogIndex(5), 3);

  EXPECT_EQ(vraft::DataIndexToLogIndex(2), 1);
  EXPECT_EQ(vraft::DataIndexToLogIndex(6), 3);
}

TEST(coding, test2) {
  uint32_t u32 = 78;
  char buf[sizeof(uint32_t)];
  vraft::EncodeFixed32(buf, u32);

  uint32_t u32_2 = vraft::DecodeFixed32(buf);
  std::cout << u32 << " " << u32_2 << std::endl;
}

TEST(coding, test3) {
  char *ptr = (char *)malloc(vraft::MetaValueBytes());

  vraft::RaftTerm term = 5;
  uint32_t entry_type = 3;
  uint32_t chk_ths = 88;
  uint32_t chk_all = 99;
  vraft::EncodeMetaValue(ptr, vraft::MetaValueBytes(), term, entry_type,
                         chk_ths, chk_all);

  vraft::RaftTerm term2;
  vraft::EntryType entry_type2;
  uint32_t chk_ths2;
  uint32_t chk_all2;
  vraft::DecodeMetaValue(ptr, vraft::MetaValueBytes(), term2, entry_type2,
                         chk_ths2, chk_all2);

  EXPECT_EQ(term, term2);
  EXPECT_EQ(vraft::U32ToEntryType(entry_type), entry_type2);
  EXPECT_EQ(chk_ths, chk_ths2);
  EXPECT_EQ(chk_all, chk_all2);

  free(ptr);
}

TEST(RaftLog, construct) {
  system("rm -rf /tmp/raftlog_test_dir");
  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    std::cout << raft_log.ToJsonString(false, false) << std::endl;
  }
  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(RaftLog, Append) {
  system("rm -rf /tmp/raftlog_test_dir");

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    for (int i = 0; i < 5; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
      std::cout << "append " << buf << ": " << raft_log.ToJsonString(true, true)
                << std::endl;
      std::cout << raft_log.ToJsonString(true, true) << std::endl;
    }

    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 5);
    EXPECT_EQ(raft_log.Append(), 6);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 5);
    EXPECT_EQ(raft_log.Append(), 6);
  }

  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(RaftLog, CheckSum) {
  system("rm -rf /tmp/raftlog_test_dir");

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    for (int i = 0; i < 10; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
      std::cout << "append " << buf << ": " << raft_log.ToJsonString(true, true)
                << std::endl;
      std::cout << raft_log.ToJsonString(true, true) << std::endl;
    }

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    raft_log.DeleteFrom(1);
    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);
  }

  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(RaftLog, DeleteFrom) {
  system("rm -rf /tmp/raftlog_test_dir");

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    for (int i = 0; i < 10; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
      std::cout << "append " << buf << ": " << raft_log.ToJsonString(true, true)
                << std::endl;
      std::cout << raft_log.ToJsonString(true, true) << std::endl;
    }

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    raft_log.DeleteFrom(7);
    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 6);
    EXPECT_EQ(raft_log.Append(), 7);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << raft_log.ToJsonString(true, true) << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 6);
    EXPECT_EQ(raft_log.Append(), 7);
  }

  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(RaftLog, DeleteFrom2) {
  system("rm -rf /tmp/raftlog_test_dir");

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    for (int i = 0; i < 10; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
    }

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    raft_log.DeleteFrom(1);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 11);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 11);

    for (int i = 0; i < 5; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
    }

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 11);
    EXPECT_EQ(raft_log.Last(), 15);
    EXPECT_EQ(raft_log.Append(), 16);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 11);
    EXPECT_EQ(raft_log.Last(), 15);
    EXPECT_EQ(raft_log.Append(), 16);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 11);
    EXPECT_EQ(raft_log.Last(), 15);
    EXPECT_EQ(raft_log.Append(), 16);

    raft_log.DeleteFrom(13);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 11);
    EXPECT_EQ(raft_log.Last(), 12);
    EXPECT_EQ(raft_log.Append(), 13);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 11);
    EXPECT_EQ(raft_log.Last(), 12);
    EXPECT_EQ(raft_log.Append(), 13);

    raft_log.DeleteFrom(0);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 13);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 13);
  }

  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(RaftLog, DeleteUtil) {
  system("rm -rf /tmp/raftlog_test_dir");

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    for (int i = 0; i < 10; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
    }

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    raft_log.DeleteUtil(5);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 6);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 6);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    raft_log.DeleteUtil(100);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 11);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 11);

    for (int i = 0; i < 5; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
    }

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 11);
    EXPECT_EQ(raft_log.Last(), 15);
    EXPECT_EQ(raft_log.Append(), 16);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 11);
    EXPECT_EQ(raft_log.Last(), 15);
    EXPECT_EQ(raft_log.Append(), 16);

    raft_log.DeleteUtil(15);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 16);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 16);
  }

  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(RaftLog, DeleteFrom_DeleteUtil) {
  system("rm -rf /tmp/raftlog_test_dir");

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    for (int i = 0; i < 10; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
    }

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    raft_log.DeleteUtil(5);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 6);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    raft_log.DeleteFrom(7);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 6);
    EXPECT_EQ(raft_log.Last(), 6);
    EXPECT_EQ(raft_log.Append(), 7);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 6);
    EXPECT_EQ(raft_log.Last(), 6);
    EXPECT_EQ(raft_log.Append(), 7);

    raft_log.DeleteUtil(6);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 7);
  }

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 7);
  }

  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(RaftLog, Get) {
  system("rm -rf /tmp/raftlog_test_dir");

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    for (int i = 0; i < 10; ++i) {
      vraft::AppendEntry entry;
      entry.term = i * 10;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
    }

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    for (vraft::RaftIndex i = 1; i <= 10; ++i) {
      vraft::LogEntry entry;
      int32_t rv = raft_log.Get(i, entry);
      EXPECT_EQ(rv, 0);
      std::cout << entry.ToJsonString(true, true) << std::endl;

      EXPECT_EQ(entry.index, i);
      EXPECT_EQ(entry.append_entry.term, (i - 1) * 10);

      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i - 1);
      EXPECT_EQ(entry.append_entry.value, std::string(buf));
    }

    {
      vraft::LogEntry entry;
      int32_t rv = raft_log.Get(11, entry);
      EXPECT_EQ(rv, -1);
    }

    raft_log.DeleteFrom(5);
    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 4);
    EXPECT_EQ(raft_log.Append(), 5);

    {
      vraft::LogEntry entry;
      int32_t rv = raft_log.Get(5, entry);
      EXPECT_EQ(rv, -1);
    }
  }

  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(RaftLog, LastEntry) {
  system("rm -rf /tmp/raftlog_test_dir");

  {
    vraft::RaftLog raft_log("/tmp/raftlog_test_dir");
    raft_log.Init();

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 0);
    EXPECT_EQ(raft_log.Last(), 0);
    EXPECT_EQ(raft_log.Append(), 1);

    {
      vraft::LogEntryPtr ptr = raft_log.LastEntry();
      assert(!ptr);
    }

    for (int i = 0; i < 10; ++i) {
      vraft::AppendEntry entry;
      entry.term = 100;
      entry.type = vraft::kData;
      char buf[32];
      snprintf(buf, sizeof(buf), "value_%d", i);
      entry.value = buf;
      raft_log.AppendOne(entry);
    }

    std::cout << "first: " << raft_log.First() << std::endl;
    std::cout << "last: " << raft_log.Last() << std::endl;
    std::cout << "append: " << raft_log.Append() << std::endl;
    EXPECT_EQ(raft_log.First(), 1);
    EXPECT_EQ(raft_log.Last(), 10);
    EXPECT_EQ(raft_log.Append(), 11);

    {
      vraft::LogEntryPtr ptr = raft_log.LastEntry();
      assert(ptr);
      EXPECT_EQ(ptr->index, 10);
      EXPECT_EQ(ptr->append_entry.term, 100);
      EXPECT_EQ(ptr->append_entry.value, std::string("value_9"));
    }
  }

  system("rm -rf /tmp/raftlog_test_dir");
}

TEST(LogEntry, test) {
  vraft::LogEntry entry;
  entry.index = 3;
  entry.append_entry.term = 100;
  entry.append_entry.type = vraft::kData;
  entry.append_entry.value = "hello";
  entry.CheckThis();
  entry.CheckAll(5);

  std::string str;
  int32_t bytes = entry.ToString(str);
  std::cout << "bytes:" << bytes << std::endl;

  std::cout << "encoding:" << std::endl;
  std::cout << entry.ToJsonString(true, true) << std::endl;
  std::cout << entry.ToJsonString(true, false) << std::endl;
  std::cout << entry.ToJsonString(false, true) << std::endl;
  std::cout << entry.ToJsonString(false, false) << std::endl;

  vraft::LogEntry entry2;
  int32_t bytes2 = entry2.FromString(str);
  assert(bytes2 > 0);
  std::cout << "bytes2:" << bytes2 << std::endl;
  EXPECT_EQ(bytes, bytes2);

  std::cout << "decoding:" << std::endl;
  std::cout << entry.ToJsonString(true, true) << std::endl;
  std::cout << entry.ToJsonString(true, false) << std::endl;
  std::cout << entry.ToJsonString(false, true) << std::endl;
  std::cout << entry.ToJsonString(false, false) << std::endl;

  EXPECT_EQ(entry.index, entry2.index);
  EXPECT_EQ(entry.chk_ths, entry2.chk_ths);
  EXPECT_EQ(entry.chk_all, entry2.chk_all);
  EXPECT_EQ(entry.append_entry.term, entry2.append_entry.term);
  EXPECT_EQ(entry.append_entry.type, entry2.append_entry.type);
  EXPECT_EQ(entry.append_entry.value, entry2.append_entry.value);
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}