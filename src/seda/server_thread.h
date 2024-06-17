#ifndef VRAFT_SERVER_THREAD_H_
#define VRAFT_SERVER_THREAD_H_

#include <vector>

#include "loop_thread.h"
#include "tcp_server.h"

namespace vraft {

class ServerThread final {
 public:
  ServerThread(const std::string &name, bool detach);
  ~ServerThread();
  ServerThread(const ServerThread &t) = delete;
  ServerThread &operator=(const ServerThread &t) = delete;

  // call in any thread
  int32_t Start();
  void Stop();
  void Join();

  // call before start
  void AddServer(TcpServerSPtr sptr);
  EventLoopSPtr LoopPtr();

 private:
  std::string name_;
  bool detach_;

 private:
  LoopThreadSPtr loop_thread_;
  std::vector<TcpServerWPtr> servers_;
};

inline ServerThread::~ServerThread() {}

}  // namespace vraft

#endif
