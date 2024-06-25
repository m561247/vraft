#include "metadata.h"

#include <gtest/gtest.h>

#include <csignal>
#include <iostream>

#include "raft.h"
#include "test_suite.h"
#include "timer.h"
#include "util.h"

//--------------------------------
// EXPECT_TRUE  true
// EXPECT_FALSE false
//
// ASSERT_TRUE  true
// ASSERT_FALSE false
//
// EXPECT_EQ  ==
// EXPECT_NE  !=
// EXPECT_NE  <
// EXPECT_LE  <=
// EXPECT_GT  >
// EXPECT_GE  >=
//
// ASSERT_EQ  ==
// ASSERT_NE  !=
// ASSERT_LT  <
// ASSERT_LE  <=
// ASSERT_GT  >
// ASSERT_GE  >=
//--------------------------------

TEST(Replica, Replica) {
  vectordb::Replica r;
  r.id = 100;
  r.name = "test-replica";
  r.path = "/tmp/data";
  r.uid = 88;
  r.table_name = "table";
  r.table_uid = 77;
  r.partition_name = "partition";
  r.partition_uid = 99;

  std::string str;
  int32_t bytes = r.ToString(str);
  std::cout << "encoding bytes:" << bytes << std::endl;
  std::cout << r.ToJsonString(false, true) << std::endl;

  vectordb::Replica r2;
  int32_t bytes2 = r2.FromString(str);
  assert(bytes2 > 0);

  std::cout << "decoding bytes:" << bytes2 << std::endl;
  std::cout << r2.ToJsonString(false, true) << std::endl;

  ASSERT_EQ(r.id, r2.id);
  ASSERT_EQ(r.name, r2.name);
  ASSERT_EQ(r.path, r2.path);
  ASSERT_EQ(r.uid, r2.uid);
  ASSERT_EQ(r.table_name, r2.table_name);
  ASSERT_EQ(r.table_uid, r2.table_uid);
  ASSERT_EQ(r.partition_name, r2.partition_name);
  ASSERT_EQ(r.partition_uid, r2.partition_uid);
}

TEST(Partition, Partition) {
  vectordb::Partition partition;
  partition.id = 100;
  partition.name = "test-partition";
  partition.path = "/tmp/data";
  partition.replica_num = 5;
  partition.uid = 88;
  for (int32_t i = 0; i < partition.replica_num; ++i) {
    std::string replica_name = vectordb::ReplicaName(partition.name, i);
    partition.replicas_by_name[replica_name] = nullptr;
    partition.replicas_by_uid[i] = nullptr;
  }

  std::string str;
  int32_t bytes = partition.ToString(str);
  std::cout << "encoding bytes:" << bytes << std::endl;
  std::cout << partition.ToJsonString(false, true) << std::endl;

  vectordb::Partition partition2;
  int32_t bytes2 = partition2.FromString(str);
  assert(bytes2 > 0);

  std::cout << "decoding bytes:" << bytes2 << std::endl;
  std::cout << partition2.ToJsonString(false, true) << std::endl;

  ASSERT_EQ(partition.id, partition2.id);
  ASSERT_EQ(partition.name, partition2.name);
  ASSERT_EQ(partition.path, partition2.path);
  ASSERT_EQ(partition.replica_num, partition2.replica_num);
  ASSERT_EQ(partition.uid, partition2.uid);
  ASSERT_EQ(partition.replicas_by_uid.size(),
            partition2.replicas_by_uid.size());
  ASSERT_EQ(partition.replicas_by_name.size(),
            partition2.replicas_by_name.size());
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}