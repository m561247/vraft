#include "request_vote.h"

#include <gtest/gtest.h>

#include <csignal>
#include <iostream>

#include "util.h"

TEST(RequestVote, test) {
  uint32_t ip32;
  bool b = vraft::StringToIpU32("127.0.0.1", ip32);
  assert(b);

  vraft::RaftAddr src(ip32, 1234, 55);
  vraft::RaftAddr dest(ip32, 5678, 99);

  vraft::RequestVote msg;
  msg.src = src;
  msg.dest = dest;
  msg.term = 77;
  msg.last_log_term = 88;
  msg.last_log_index = 99;

  std::string msg_str;
  int32_t bytes = msg.ToString(msg_str);
  std::cout << "bytes:" << bytes << std::endl;

  std::cout << "encoding:" << std::endl;
  std::cout << msg.ToJsonString(true, true) << std::endl;
  std::cout << msg.ToJsonString(true, false) << std::endl;
  std::cout << msg.ToJsonString(false, true) << std::endl;
  std::cout << msg.ToJsonString(false, false) << std::endl;

  vraft::RequestVote msg2;
  b = msg2.FromString(msg_str);
  assert(b);

  std::cout << msg2.ToJsonString(true, true) << std::endl;
  std::cout << msg2.ToJsonString(true, false) << std::endl;
  std::cout << msg2.ToJsonString(false, true) << std::endl;
  std::cout << msg2.ToJsonString(false, false) << std::endl;

  std::cout << "decoding:" << std::endl;
  EXPECT_EQ(msg.src.ToU64(), msg2.src.ToU64());
  EXPECT_EQ(msg.dest.ToU64(), msg2.dest.ToU64());
  EXPECT_EQ(msg.term, msg2.term);
  EXPECT_EQ(msg.last_log_term, msg2.last_log_term);
  EXPECT_EQ(msg.last_log_index, msg2.last_log_index);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}