#include "echo_server.h"

#include <csignal>
#include <iostream>
#include <thread>

EchoServerPtr server;

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << std::endl;
  if (server) {
    server->Stop();
  }
}

int main(int argc, char **argv) {
#if 0
  vraft::LoggerOptions logger_options{"echo_server", false, 1, 8192,
                                      vraft::kLoggerTrace};
  vraft::vraft_logger.Init("/tmp/echo_server.log", logger_options);
#endif
  std::signal(SIGINT, SignalHandler);
  // std::signal(SIGSEGV, SignalHandler);

  vraft::TcpOptions opt = {true};
  vraft::HostPort listen_addr("127.0.0.1", 9988);
  server = std::make_shared<EchoServer>(listen_addr, opt);

  std::cout << "echo-server start, listening on " << listen_addr.ToString()
            << " ..." << std::endl;

  EchoServerPtr ptr = server;
  std::thread t([ptr] { ptr->Start(); });
  t.join();

  std::cout << "echo-server stop ..." << std::endl;
  // vraft::Logger::ShutDown();
  return 0;
}
