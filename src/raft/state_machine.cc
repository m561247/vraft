#include "state_machine.h"

namespace vraft {

StateMachine::StateMachine(std::string path) : path_(path) {}

StateMachine::~StateMachine() {}

int32_t StateMachine::Init() { return Restore(); }

}  // namespace vraft
