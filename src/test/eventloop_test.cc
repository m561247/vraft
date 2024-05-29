#include "eventloop.h"

#include <gtest/gtest.h>

#include <csignal>
#include <iostream>
#include <thread>

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

TEST(EventLoop, EventLoop) {
  system("rm -f /tmp/eventloop_test.log");
  vraft::LoggerOptions o;
  o.logger_name = "EventLoop.EventLoop";
  vraft::vraft_logger.Init("/tmp/eventloop_test.log", o);

  vraft::EventLoop loop("test_loop");
  vraft::EventLoop *l = &loop;
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