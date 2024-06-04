#include "echo_server.h"

#include <csignal>
#include <iostream>
#include <thread>

EchoServerWPtr weak_server;

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << std::endl;
  auto sptr = weak_server.lock();
  if (sptr) {
    sptr->Stop();
  }
}

int main(int argc, char **argv) {
  vraft::LoggerOptions logger_options{"echo-server", false, 1, 8192,
                                      vraft::kLoggerTrace};
  vraft::vraft_logger.Init("/tmp/echo_server.log", logger_options);

  std::signal(SIGINT, SignalHandler);
  // std::signal(SIGSEGV, SignalHandler);

  vraft::TcpOptions opt = {true};
  vraft::HostPort listen_addr("127.0.0.1", 9988);
  EchoServerSPtr server = std::make_shared<EchoServer>(listen_addr, opt);

  std::cout << "echo-server start, listening on " << listen_addr.ToString()
            << " ..." << std::endl;

  std::thread t([server] { server->Start(); });
  weak_server = server;
  t.join();

  std::cout << "echo-server stop ..." << std::endl;
  vraft::Logger::ShutDown();
  return 0;
}
