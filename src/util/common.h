#ifndef VRAFT_COMMON_H_
#define VRAFT_COMMON_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace vraft {

#define JSON_TAB 2

using RaftIndex = uint32_t;
using RaftTerm = uint64_t;

using Functor = std::function<void()>;
using FunctorFinish = std::function<void(int32_t)>;

using TracerCb = std::function<void(std::string)>;

using SendFunc =
    std::function<int32_t(uint64_t dest, const char *buf, unsigned int size)>;

class EventLoop;
using EventLoopSPtr = std::shared_ptr<EventLoop>;
using EventLoopUPtr = std::unique_ptr<EventLoop>;
using EventLoopWPtr = std::weak_ptr<EventLoop>;

class AsyncQueue;
using AsyncQueueSPtr = std::shared_ptr<AsyncQueue>;
using AsyncQueueUPtr = std::unique_ptr<AsyncQueue>;
using AsyncQueueWPtr = std::weak_ptr<AsyncQueue>;

class AsyncStop;
using AsyncStopSPtr = std::shared_ptr<AsyncStop>;
using AsyncStopUPtr = std::unique_ptr<AsyncStop>;
using AsyncStopWPtr = std::weak_ptr<AsyncStop>;

class Timer;
using TimerSPtr = std::shared_ptr<Timer>;
using TimerUPtr = std::unique_ptr<Timer>;
using TimerWPtr = std::weak_ptr<Timer>;

using TimerId = int64_t;
using TimerMap = std::map<TimerId, TimerSPtr>;
using TimerFunctor = std::function<void(Timer *)>;

class TcpConnection;
using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;
using TcpConnectionUPtr = std::unique_ptr<TcpConnection>;
using TcpConnectionWPtr = std::weak_ptr<TcpConnection>;

class TcpServer;
using TcpServerSPtr = std::shared_ptr<TcpServer>;
using TcpServerUPtr = std::unique_ptr<TcpServer>;
using TcpServerWPtr = std::weak_ptr<TcpServer>;

class TcpClient;
using TcpClientSPtr = std::shared_ptr<TcpClient>;
using TcpClientUPtr = std::unique_ptr<TcpClient>;
using TcpClientWPtr = std::weak_ptr<TcpClient>;

class RaftServer;
using RaftServerSPtr = std::shared_ptr<RaftServer>;
using RaftServerUPtr = std::unique_ptr<RaftServer>;
using RaftServerWPtr = std::weak_ptr<RaftServer>;

class Remu;
using RemuSPtr = std::shared_ptr<Remu>;
using RemuUPtr = std::unique_ptr<Remu>;
using RemuWPtr = std::weak_ptr<Remu>;

class Raft;
using RaftSPtr = std::shared_ptr<Raft>;
using RaftUPtr = std::unique_ptr<Raft>;
using RaftWPtr = std::weak_ptr<Raft>;

}  // namespace vraft

#endif
