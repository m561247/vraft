#include "remu.h"

#include <cstdio>

#include "clock.h"
#include "raft_server.h"
#include "util.h"

namespace vraft {

void Remu::Print(bool tiny, bool one_line) {
  printf("---remu--- %s ---:\n", NsToString(Clock::NSec()).c_str());
  for (auto ptr : raft_servers) {
    ptr->Print(tiny, one_line);
    if (!one_line) {
      printf("\n");
    }
  }
  printf("\n");
  fflush(nullptr);
}

void Remu::Create() {
  for (auto conf : configs) {
    auto sptr = loop.lock();
    assert(sptr);
    vraft::RaftServerSPtr ptr = std::make_shared<vraft::RaftServer>(sptr, conf);
    raft_servers.push_back(ptr);
  }
}

void Remu::Start() {
  for (auto &ptr : raft_servers) {
    if (ptr) {
      ptr->Start();
    }
  }
}

void Remu::Stop() {
  for (auto &ptr : raft_servers) {
    if (ptr) {
      ptr->Stop();
    }
  }
}

void Remu::Clear() { raft_servers.clear(); }

}  // namespace vraft
