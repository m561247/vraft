#include "tcp_connection.h"

#include <gtest/gtest.h>

#include <atomic>
#include <csignal>
#include <iostream>

#include "eventloop.h"
#include "raft.h"
#include "test_suite.h"
#include "timer.h"
#include "util.h"
#include "uv_wrapper.h"

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

vraft::EventLoop *lptr;
vraft::UvTcpUPtr *uptr;
std::atomic<bool> b(false);

void ConnCb(vraft::UvConnect *req, int status) { b.store(true); }

void ConnectLoop() {
  vraft::UvConnect req;
  vraft::HostPort hp("baidu.com:80");
  int32_t rv = vraft::UvTcpConnect(&req, uptr->get(),
                                   (const struct sockaddr *)&hp.addr, ConnCb);
  assert(rv == 0);
  lptr->Loop();
}

TEST(TcpConnection, TcpConnection) {
  system("rm -f /tmp/tcp_connection_test.log");
  vraft::LoggerOptions o;
  o.logger_name = "TcpConnection.TcpConnection";
  vraft::vraft_logger.Init("/tmp/tcp_connection_test.log", o);

  vraft::EventLoop loop("test_loop");
  lptr = &loop;

  vraft::UvTcpUPtr ptr = std::make_unique<vraft::UvTcp>();
  uptr = &ptr;

  int32_t rv = vraft::UvTcpInit(lptr->UvLoopPtr(), uptr->get());
  assert(rv == 0);

  vraft::EventLoop *l = lptr;
  std::thread t(ConnectLoop);

  std::cout << "wait 3s for connect to baidu ..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));

// lambda cannot pass valgrind, sad!!
#if 0
  std::thread t2([] {
    while (!b.load()) {
      ;
    }
  });
  t2.join();
#endif

  vraft::TcpConnection conn(l, "test_conn", std::move(ptr));
  vraft::TcpConnection *pc = &conn;
  std::cout << "name: " << conn.name() << std::endl;
  std::cout << "local_addr: " << conn.local_addr().ToString() << std::endl;
  std::cout << "peer_addr: " << conn.peer_addr().ToString() << std::endl;

  l->RunFunctor([pc]() { pc->Close(); });
  std::thread t3([l]() {
    std::cout << "after 3s, call loop stop() ..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    l->Stop();
  });
  t.join();
  t3.join();
  std::cout << "loop stop" << std::endl;
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}