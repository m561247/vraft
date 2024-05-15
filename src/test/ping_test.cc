#include "ping.h"
#include "util.h"
#include <csignal>
#include <gtest/gtest.h>
#include <iostream>

TEST(PingTests, Coding) {
  uint32_t ip32;
  bool b = vraft::StringToIpU32("127.0.0.1", ip32);
  assert(b);

  vraft::RaftAddr src(ip32, 1234, 55);
  vraft::RaftAddr dest(ip32, 5678, 99);

  vraft::Ping msg;
  msg.msg = "ping";
  msg.src = src;
  msg.dest = dest;
  std::cout << "msg: " << msg.msg << " " << msg.src.ToU64() << " "
            << msg.dest.ToU64() << std::endl;

  std::string msg_str;
  int32_t bytes = msg.ToString(msg_str);
  std::cout << bytes << std::endl;

  vraft::Ping msg2;
  b = msg2.FromString(msg_str);
  assert(b);
  std::cout << "msg2: " << msg2.msg << " " << msg2.src.ToU64() << " "
            << msg2.dest.ToU64() << std::endl;

  EXPECT_EQ(msg.msg, msg2.msg);
  EXPECT_EQ(msg.src.ToU64(), msg2.src.ToU64());
  EXPECT_EQ(msg.dest.ToU64(), msg2.dest.ToU64());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}