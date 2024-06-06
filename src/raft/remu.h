#ifndef VRAFT_REMU_H_
#define VRAFT_REMU_H_

#include <vector>

#include "common.h"
#include "config.h"

namespace vraft {

// raft emulator, good!!
struct Remu {
  Remu(EventLoopSPtr &l) : loop(l) {}

  EventLoopWPtr loop;
  std::vector<vraft::Config> configs;
  std::vector<vraft::RaftServerSPtr> raft_servers;

  void Create();
  void Start();
  void Stop();
  void Clear();

  void Log();
  void Print(bool tiny = true, bool one_line = true);
};

}  // namespace vraft

#endif
