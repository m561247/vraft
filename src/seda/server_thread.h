#ifndef VRAFT_SERVER_THREAD_H_
#define VRAFT_SERVER_THREAD_H_

namespace vraft {

class ServerThread final {
 public:
  ServerThread();
  ~ServerThread();
  ServerThread(const ServerThread &t) = delete;
  ServerThread &operator=(const ServerThread &t) = delete;

 private:
};

inline ServerThread::ServerThread() {}

inline ServerThread::~ServerThread() {}

}  // namespace vraft

#endif
