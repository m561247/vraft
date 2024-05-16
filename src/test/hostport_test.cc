#include "hostport.h"

#include <gtest/gtest.h>

#include <csignal>
#include <iostream>

#include "util.h"

TEST(HostPortTests, construct) {
  vraft::HostPort hp("127.0.0.1", 8899);
  std::cout << hp.ToString() << std::endl;
  EXPECT_EQ(hp.ToString(), std::string("127.0.0.1:8899"));
}

TEST(HostPortTests, HostPortToSockaddrIn) {
  vraft::HostPort hp("127.0.0.1", 8899);
  std::cout << hp.ToString() << std::endl;

  sockaddr_in out_sockaddr_in;
  bool b = vraft::HostPortToSockaddrIn(hp.host, hp.port, out_sockaddr_in);
  EXPECT_EQ(b, true);

  std::cout << "ip:" << out_sockaddr_in.sin_addr.s_addr << std::endl;
  std::cout << "port:" << out_sockaddr_in.sin_port << std::endl;

  std::cout << "-----"
            << vraft::IpU32ToIpString(out_sockaddr_in.sin_addr.s_addr)
            << std::endl;

  vraft::HostPort hp2 = vraft::SockaddrInToHostPort(&out_sockaddr_in);
  std::cout << hp2.ToString() << std::endl;

  EXPECT_EQ(hp.host, hp2.host);
  EXPECT_EQ(hp.port, hp2.port);
  EXPECT_EQ(hp.convert_ok, hp2.convert_ok);
  // EXPECT_EQ(hp.addr, hp2.addr);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}