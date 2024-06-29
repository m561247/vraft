#ifndef VRAFT_TURING_MACHINE_H_
#define VRAFT_TURING_MACHINE_H_

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "eventloop.h"

namespace vraft {

#define INIT_POS 10

char characters[] = {'0', '1'};

enum HeadState {
  kHeadState0 = 0,
  kHeadState1,
  kHeadState2,
  kHeadState3,
};

class Head final {
 public:
  explicit Head(HeadState current_state, int32_t current_pos);
  ~Head();
  Head(const Head &) = delete;
  Head &operator=(const Head &) = delete;

  char Read();
  void Write(char c);

  std::string ToString();

 private:
  HeadState current_state_;
  int32_t current_pos_;
  std::vector<char> tape_;
};

inline Head::Head(HeadState current_state, int32_t current_pos)
    : current_state_(current_state),
      current_pos_(current_pos),
      tape_(INIT_POS * 2, '0') {}

inline Head::~Head() {}

struct RulesKey {
  HeadState current_state;
  char read_char;
};

struct RulesValue {
  int32_t pos;
  HeadState next_state;
  char write_char;
};

void timer_cb(Timer *timer);

class TuringMachine final {
 public:
  explicit TuringMachine();
  ~TuringMachine();
  TuringMachine(const TuringMachine &) = delete;
  TuringMachine &operator=(const TuringMachine &) = delete;

  void Run();
  void DoEvent();

 private:
  void InitRules();

 private:
  std::map<RulesKey, RulesValue> rules_;
  Head head_;
  EventLoopSPtr loop_;
  int32_t step_;
};

inline TuringMachine::~TuringMachine() {}

}  // namespace vraft

#endif
