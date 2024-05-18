#include "tracer.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>

#include "coding.h"
#include "ping.h"
#include "raft.h"

TEST(Tracer, test) {
  system("rm -rf /tmp/raft_tracer_test");
  vraft::RaftConfig rc;
  rc.me = vraft::RaftAddr("127.0.0.1", 9000, 100);
  for (uint16_t i = 1; i <= 2; ++i) {
    uint16_t port = 9000 + i;
    rc.peers.push_back(vraft::RaftAddr("127.0.0.1", port, 100));
  }
  system("mkdir -p /tmp/raft_tracer_test");
  vraft::Raft r("/tmp/raft_tracer_test", rc);

  vraft::RaftAddr src("127.0.0.1", 1234, 55);
  vraft::RaftAddr dest("127.0.0.1", 5678, 99);

  vraft::Ping msg;
  msg.src = src;
  msg.dest = dest;
  msg.msg = "ping";

  std::string event_str = msg.ToJsonString(false, true);
  r.tracer.Init();
  r.tracer.PrepareState0();
  r.tracer.PrepareEvent(vraft::kRecv, event_str);
  r.tracer.PrepareState1();
  std::cout << r.tracer.Finish() << std::endl;
  system("rm -rf /tmp/raft_tracer_test");
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}