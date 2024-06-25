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
  partition.table_name = "test-table";
  partition.table_uid = 77;
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
  ASSERT_EQ(partition.table_name, partition2.table_name);
  ASSERT_EQ(partition.table_uid, partition2.table_uid);
  ASSERT_EQ(partition.replicas_by_uid.size(),
            partition2.replicas_by_uid.size());
  ASSERT_EQ(partition.replicas_by_name.size(),
            partition2.replicas_by_name.size());
}

TEST(Table, Table) {
  vectordb::Table table;
  table.name = "test-table";
  table.path = "/tmp/data";
  table.partition_num = 5;
  table.replica_num = 3;
  table.dim = 1024;
  table.uid = 88;
  for (int32_t i = 0; i < table.partition_num; ++i) {
    std::string partition_name = vectordb::PartitionName(table.name, i);
    table.partitions_by_name[partition_name] = nullptr;
    table.partitions_by_uid[i] = nullptr;
  }

  std::string str;
  int32_t bytes = table.ToString(str);
  std::cout << "encoding bytes:" << bytes << std::endl;
  std::cout << table.ToJsonString(false, true) << std::endl;

  vectordb::Table table2;
  int32_t bytes2 = table2.FromString(str);
  assert(bytes2 > 0);

  std::cout << "decoding bytes:" << bytes2 << std::endl;
  std::cout << table2.ToJsonString(false, true) << std::endl;

  ASSERT_EQ(table.name, table2.name);
  ASSERT_EQ(table.path, table2.path);
  ASSERT_EQ(table.partition_num, table2.partition_num);
  ASSERT_EQ(table.replica_num, table2.replica_num);
  ASSERT_EQ(table.dim, table2.dim);
  ASSERT_EQ(table.uid, table2.uid);
  ASSERT_EQ(table.partitions_by_uid.size(), table2.partitions_by_uid.size());
  ASSERT_EQ(table.partitions_by_name.size(), table2.partitions_by_name.size());
}

TEST(Metadata, AddTable) {
  vectordb::Metadata meta("xx");

  for (int32_t i = 0; i < 3; ++i) {
    vectordb::Table table;
    char buf[128];
    snprintf(buf, sizeof(buf), "table_%d", i);
    table.name = buf;

    snprintf(buf, sizeof(buf), "/tmp/data/%s", table.name.c_str());
    table.path = buf;

    table.partition_num = 10;
    table.replica_num = 3;
    table.dim = 1024;
    table.uid = 0;
    int32_t rv = meta.AddTable(table);
    ASSERT_EQ(rv, 0);
  }

  std::cout << meta.ToJsonString(false, false) << std::endl;
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}