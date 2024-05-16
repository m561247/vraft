#ifndef VRAFT_RAFT_H_
#define VRAFT_RAFT_H_

#include <vector>

#include "config.h"
#include "ping.h"
#include "ping_reply.h"
#include "timer.h"

namespace vraft {

class RaftServer;

void SendPing(Timer *timer);

class Raft final {
 public:
  Raft(RaftServer *raft_server);
  ~Raft();
  Raft(const Raft &t) = delete;
  Raft &operator=(const Raft &t) = delete;

  int32_t Start();
  int32_t Stop();

  int32_t OnPing(struct Ping &msg);
  int32_t OnPingReply(struct PingReply &msg);

  int16_t id() { return id_; }
  HostPort my_addr() { return my_addr_; }
  std::vector<HostPort> &peers() { return peers_; }

 private:
  RaftServer *raft_server_;

  int16_t id_;
  HostPort my_addr_;
  std::vector<HostPort> peers_;

  TimerPtr ping_timer_;
  TimerPtr election_timer_;
  TimerPtr heartbeat_timer_;

  friend void SendPing(Timer *timer);
};

inline Raft::~Raft() {}

}  // namespace vraft

#endif
