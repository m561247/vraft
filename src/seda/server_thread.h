#ifndef VRAFT_SERVER_THREAD_H_
#define VRAFT_SERVER_THREAD_H_

#include <vector>

#include "loop_thread.h"
#include "tcp_server.h"

namespace vraft {

class ServerThread final {
 public:
  ServerThread(int32_t server_num);
  ~ServerThread();
  ServerThread(const ServerThread &t) = delete;
  ServerThread &operator=(const ServerThread &t) = delete;

 private:
  int32_t server_num_;

  LoopThreadSPtr loop_thread_;
  std::vector<TcpServerSPtr> servers_;
};

inline ServerThread::~ServerThread() {}

}  // namespace vraft

#endif
