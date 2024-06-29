#include "turing_machine.h"

namespace vraft {

char Head::Read() { return tape_[current_pos_]; }

void Head::Write(char c) { tape_[current_pos_] = c; }

std::string Head::ToString() {
  std::string s;
  char buf[1024];
  snprintf(buf, sizeof(buf), "current_state: %d\n", current_state_);
  s.append(buf);
  snprintf(buf, sizeof(buf), "current_pos: %d\n", current_pos_);
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
  InitRules();

  loop_ = std::make_shared<EventLoop>("turing-machine");
  loop_->Init();
}

void TuringMachine::InitRules() {}

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
  std::cout << "step: " << step_ << std::endl;
  std::cout << head_.ToString() << std::endl;
  ++step_;
}

}  // namespace vraft

int main(int argc, char **argv) {
  vraft::TuringMachine tm;

  tm.Run();
  return 0;
}