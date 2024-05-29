#include "echo_server.h"
#include <iostream>
#include <csignal>

EchoServerPtr server;

void SignalHandler(int signal) {
  if (server) {
    server->Stop();
  }
  vraft::Logger::ShutDown();
  exit(signal);
}

int main(int argc, char **argv) {
  vraft::LoggerOptions logger_options{"echo_server", false, 1, 8192,
                                      vraft::kLoggerTrace};
  vraft::vraft_logger.Init("/tmp/echo_server.log", logger_options);

  std::signal(SIGINT, SignalHandler);
  // std::signal(SIGSEGV, SignalHandler);

  vraft::TcpOptions opt = {true};
  vraft::HostPort listen_addr("127.0.0.1", 9988);
  server = std::make_shared<EchoServer>(listen_addr, opt);

  std::cout << "echo-server start, listening on " << listen_addr.ToString() << " ..." << std::endl;
  server->Start();

  return 0;
}
