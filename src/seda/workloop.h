#ifndef VRAFT_WORKLOOP_H_
#define VRAFT_WORKLOOP_H_

namespace vraft {

class WorkLoop {
 public:
  WorkLoop() {}
  ~WorkLoop() {}
  WorkLoop(const WorkLoop &wl) = delete;
  WorkLoop &operator=(const WorkLoop &wl) = delete;

 private:
};

}  // namespace vraft

#endif