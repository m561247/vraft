#ifndef VRAFT_CLIENT_THREAD_H_
#define VRAFT_CLIENT_THREAD_H_

namespace vraft {

class ClientThread final {
 public:
  ClientThread();
  ~ClientThread();
  ClientThread(const ClientThread &t) = delete;
  ClientThread &operator=(const ClientThread &t) = delete;

 private:
};

inline ClientThread::ClientThread() {}

inline ClientThread::~ClientThread() {}

}  // namespace vraft

#endif
