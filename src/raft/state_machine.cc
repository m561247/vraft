#include "state_machine.h"

namespace vraft {

StateMachine::StateMachine(std::string path) : path_(path) {}

StateMachine::~StateMachine() {}

}  // namespace vraft
