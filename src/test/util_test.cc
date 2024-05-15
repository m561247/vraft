#include "util.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(UTIL, MISC) { std::cout << vraft::IpU32ToIpString(32765) << std::endl; }

TEST(UTIL, ToLower) {
  std::string s = "AAAaaa";
  vraft::ToLower(s);
  EXPECT_EQ(s, std::string("aaaaaa"));
}

TEST(UTIL, HostNameToIpU32) {
  uint32_t ip32;
  bool b = vraft::HostNameToIpU32("baidu.com", ip32);
  EXPECT_EQ(b, true);
  std::cout << ip32 << std::endl;

  std::string ip_str = vraft::IpU32ToIpString(ip32);
  std::cout << ip_str << std::endl;
  // EXPECT_EQ(ip_str, std::string("110.242.68.66"));
}

TEST(UTIL, IpStringToIpU32) {
  uint32_t ip32;
  bool b = vraft::IpStringToIpU32("1.2.3.4", ip32);
  EXPECT_EQ(b, true);
  std::cout << ip32 << std::endl;

  std::string ip_str = vraft::IpU32ToIpString(ip32);
  std::cout << ip_str << std::endl;
  EXPECT_EQ(ip_str, std::string("1.2.3.4"));
}

TEST(UTIL, StringToIpU32) {
  uint32_t ip32;
  bool b = vraft::StringToIpU32("baidu.com", ip32);
  EXPECT_EQ(b, true);
  std::cout << ip32 << std::endl;

  std::string ip_str = vraft::IpU32ToIpString(ip32);
  std::cout << ip_str << std::endl;
  // EXPECT_EQ(ip_str, std::string("110.242.68.66"));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}