#include "raft.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>

#include "coding.h"
#include "ping.h"
#include "ping_reply.h"
#include "raft.h"
#include "tracer.h"

TEST(Raft, test) {
  system("rm -rf /tmp/raft_load_test");
  vraft::RaftConfig rc;
  rc.me = vraft::RaftAddr("127.0.0.1", 9000, 100);
  for (uint16_t i = 1; i <= 2; ++i) {
    uint16_t port = 9000 + i;
    rc.peers.push_back(vraft::RaftAddr("127.0.0.1", port, 100));
  }
  vraft::Raft r("/tmp/raft_load_test", rc);
  r.Init();
  EXPECT_EQ(r.Id(), 100);
  EXPECT_EQ(r.Peers().size(), static_cast<size_t>(2));
  std::cout << r.ToJsonString(false, false) << std::endl;
  system("rm -rf /tmp/raft_load_test");
}

TEST(Raft, test2) {
  system("rm -rf /tmp/raft_load_test");
  {
    vraft::RaftConfig rc;
    rc.me = vraft::RaftAddr("127.0.0.1", 9000, 100);
    for (uint16_t i = 1; i <= 2; ++i) {
      uint16_t port = 9000 + i;
      rc.peers.push_back(vraft::RaftAddr("127.0.0.1", port, 100));
    }
    vraft::Raft r("/tmp/raft_load_test", rc);
    r.Init();
    EXPECT_EQ(r.Id(), 100);
    EXPECT_EQ(r.Peers().size(), static_cast<size_t>(2));
    std::cout << r.ToJsonString(false, false) << std::endl;
  }

  {
    vraft::RaftConfig rc;
    rc.me = vraft::RaftAddr("1.2.3.4", 77, 88);
    vraft::Raft r("/tmp/raft_load_test", rc);
    r.Init();
    EXPECT_EQ(r.Id(), 100);
    EXPECT_EQ(r.Peers().size(), static_cast<size_t>(2));
    std::cout << r.ToJsonString(false, false) << std::endl;
  }

  system("rm -rf /tmp/raft_load_test");
  {
    vraft::RaftConfig rc;
    rc.me = vraft::RaftAddr("1.2.3.4", 77, 88);
    vraft::Raft r("/tmp/raft_load_test", rc);
    r.Init();
    EXPECT_EQ(r.Id(), 88);
    EXPECT_EQ(r.Peers().size(), static_cast<size_t>(0));
    std::cout << r.ToJsonString(false, false) << std::endl;
  }

  system("rm -rf /tmp/raft_load_test");
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}