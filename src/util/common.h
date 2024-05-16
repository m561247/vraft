#ifndef VRAFT_COMMON_H_
#define VRAFT_COMMON_H_

#include <functional>

namespace vraft {

#define JSON_TAB 2

using Functor = std::function<void()>;
using FunctorFinish = std::function<void(int32_t)>;

}  // namespace vraft

#endif
