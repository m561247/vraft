#ifndef VRAFT_COMMON_H_
#define VRAFT_COMMON_H_

#include <functional>

namespace vraft {

#define JSON_TAB 2

using RaftIndex = uint32_t;
using RaftTerm = uint64_t;

using Functor = std::function<void()>;
using FunctorFinish = std::function<void(int32_t)>;

using SendFunc =
    std::function<int32_t(uint64_t dest, const char *buf, unsigned int size)>;

}  // namespace vraft

#endif
