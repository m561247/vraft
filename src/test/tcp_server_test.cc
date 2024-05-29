#include <gtest/gtest.h>

#include <csignal>
#include <iostream>
#include <thread>

#include "eventloop.h"
#include "raft.h"
#include "test_suite.h"
#include "timer.h"
#include "util.h"
#include "tcp_server.h"

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

TEST(TcpServer, TcpServer) {
  system("rm -f /tmp/tcpserver_test.log");
  vraft::LoggerOptions o;
  o.logger_name = "TcpServer.TcpServer";
  vraft::vraft_logger.Init("/tmp/tcpserver_test.log", o);

  vraft::EventLoop loop("test_loop");
  vraft::EventLoop *l = &loop;

  vraft::HostPort hp("127.0.0.1:9988");
  vraft::TcpOptions to;
  vraft::TcpServer tcp_server(hp, "tcp_server", to, l);
  vraft::TcpServer *pt = &tcp_server;
  l->RunFunctor([pt]() { pt->Stop(); });

  std::thread t([l]() { l->Loop(); });
  std::thread t2([l]() {
    std::cout << "after 3s, call loop stop() ..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    l->Stop();
  });
  t.join();
  t2.join();
  std::cout << "loop stop" << std::endl;
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}