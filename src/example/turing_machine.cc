#include "turing_machine.h"

namespace vraft {

char Head::Read() { return tape_[pos_]; }

void Head::Write(char c) { tape_[pos_] = c; }

void Head::MoveLeft() { pos_--; }

void Head::MoveRight() { pos_++; }

std::string Head::ToString() {
  std::string s;
  char buf[1024];
  snprintf(buf, sizeof(buf), "current_state: %d\n", state_);
  s.append(buf);
  snprintf(buf, sizeof(buf), "current_pos: %d\n", pos_);
  s.append(buf);
  s.append("tape: ");
  for (size_t i = 0; i < tape_.size(); ++i) {
    snprintf(buf, sizeof(buf), "%c ", tape_[i]);
    s.append(buf);
  }
  s.append("\n");
  return s;
}

void timer_cb(Timer *timer) {
  TuringMachine *tm = reinterpret_cast<TuringMachine *>(timer->data());
  tm->DoEvent();
}

TuringMachine::TuringMachine() : head_(kHeadState0, INIT_POS), step_(0) {
  // init tape
  int32_t pos = head_.pos();
  head_.tape()[pos++] = '1';
  head_.tape()[pos++] = '1';
  head_.tape()[pos++] = '1';
  head_.tape()[pos++] = '1';
  head_.tape()[pos++] = '0';
  head_.tape()[pos++] = '1';
  head_.tape()[pos++] = '1';
  head_.tape()[pos++] = '1';

  // init rules
  InitRules();

  loop_ = std::make_shared<EventLoop>("turing-machine");
  loop_->Init();
}

void TuringMachine::InitRules() {
  rules_[RuleKey{kHeadState0, '1'}] =
      RuleValue{'1', kDirectionRight, kHeadState0};

  rules_[RuleKey{kHeadState0, '0'}] =
      RuleValue{'1', kDirectionRight, kHeadState1};

  rules_[RuleKey{kHeadState1, '1'}] =
      RuleValue{'1', kDirectionRight, kHeadState1};

  rules_[RuleKey{kHeadState1, '0'}] =
      RuleValue{'0', kDirectionLeft, kHeadState2};

  rules_[RuleKey{kHeadState2, '1'}] =
      RuleValue{'0', kDirectionHalt, kHeadState2};

  rules_[RuleKey{kHeadState2, '0'}] =
      RuleValue{'0', kDirectionHalt, kHeadState2};
}

void TuringMachine::Run() {
  TimerParam param;
  param.timeout_ms = 1000;
  param.repeat_ms = 1000;
  param.cb = timer_cb;
  param.data = this;
  loop_->AddTimer(param);
  loop_->Loop();
}

void TuringMachine::DoEvent() {
  std::cout << "step: " << step_++ << std::endl;
  std::cout << head_.ToString() << std::endl;

  // read char
  char read_ch = head_.Read();
  std::cout << "[event] read:" << read_ch << std::endl << std::endl;

  // find rule
  RuleKey rule_key = {head_.state(), read_ch};
  auto it = rules_.find(rule_key);
  assert(it != rules_.end());
  RuleValue rule_value = it->second;

  // next state
  head_.Write(rule_value.write_char);
  if (rule_value.direction == kDirectionLeft) {
    head_.MoveLeft();
  } else if (rule_value.direction == kDirectionRight) {
    head_.MoveRight();
  } else if (rule_value.direction == kDirectionHalt) {
    ;
  } else {
    assert(0);
  }
  head_.set_state(rule_value.next_state);
}

}  // namespace vraft

int main(int argc, char **argv) {
  vraft::TuringMachine tm;

  tm.Run();
  return 0;
}
