#include "test_suite.h"

#include "raft.h"
#include "timer.h"

namespace vraft {

TimerFunctor timer_func;
TestState current_state = kTestState0;
std::unordered_map<TestState, StateChange> rules;

bool HasLeader() { return true; }

void InitRemuTest() {
  rules[kTestState0].next = kTestStateEnd;
  rules[kTestState0].func = HasLeader;
}

void TestTick(Timer *timer) {
  if (timer_func) {
    timer_func(timer);
  }

  switch (current_state) {
    case kTestState0: {
      if (rules[current_state].func && rules[current_state].func()) {
        current_state = rules[current_state].next;
      }
      break;
    }

    case kTestStateEnd: {
      exit(0);
    }

    default:
      assert(0);
  }
}

}  // namespace vraft
