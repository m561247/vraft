#include "append_entries_reply.h"

#include <gtest/gtest.h>

#include <csignal>
#include <iostream>

#include "util.h"

TEST(AppendEntriesReply, test) {
  uint32_t ip32;
  bool b = vraft::StringToIpU32("127.0.0.1", ip32);
  assert(b);

  vraft::RaftAddr src(ip32, 1234, 55);
  vraft::RaftAddr dest(ip32, 5678, 99);

  vraft::AppendEntriesReply msg;
  msg.src = src;
  msg.dest = dest;
  msg.term = 77;
  msg.success = true;
  msg.num_entries = 123;
  msg.term = 100;

  std::string msg_str;
  int32_t bytes = msg.ToString(msg_str);
  std::cout << "bytes:" << bytes << std::endl;

  std::cout << "encoding:" << std::endl;
  std::cout << msg.ToJsonString(true, true) << std::endl;
  std::cout << msg.ToJsonString(true, false) << std::endl;
  std::cout << msg.ToJsonString(false, true) << std::endl;
  std::cout << msg.ToJsonString(false, false) << std::endl;

  vraft::AppendEntriesReply msg2;
  b = msg2.FromString(msg_str);
  assert(b);

  std::cout << "decoding:" << std::endl;
  std::cout << msg2.ToJsonString(true, true) << std::endl;
  std::cout << msg2.ToJsonString(true, false) << std::endl;
  std::cout << msg2.ToJsonString(false, true) << std::endl;
  std::cout << msg2.ToJsonString(false, false) << std::endl;

  EXPECT_EQ(msg.src.ToU64(), msg2.src.ToU64());
  EXPECT_EQ(msg.dest.ToU64(), msg2.dest.ToU64());
  EXPECT_EQ(msg.term, msg2.term);
  EXPECT_EQ(msg.success, msg2.success);
  EXPECT_EQ(msg.last_log_index, msg2.last_log_index);
  EXPECT_EQ(msg.num_entries, msg2.num_entries);
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}