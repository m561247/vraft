#include "echo_client.h"

#include <csignal>
#include <iostream>
#include <thread>

vraft::TcpClientSPtr client;
vraft::EventLoopSPtr loop;

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << std::endl;
  client->Stop();
  // loop.Stop();
}

int main(int argc, char **argv) {
#if 0
  vraft::LoggerOptions logger_options{"echo-client", false, 1, 8192,
                                      vraft::kLoggerTrace};
  vraft::vraft_logger.Init("/tmp/echo_client.log", logger_options);
#endif

  loop = std::make_shared<vraft::EventLoop>("echo_client");

  std::signal(SIGINT, SignalHandler);
  // std::signal(SIGSEGV, SignalHandler);

  vraft::TcpOptions opt = {true};
  vraft::HostPort dest_addr("127.0.0.1", 9988);

  client =
      std::make_shared<vraft::TcpClient>(loop, "echo_client", dest_addr, opt);
  client->set_on_connection_cb(std::bind(&OnConnection, std::placeholders::_1));
  client->TimerConnect(100);

#if 0
  vraft::EventLoop *l = &loop;
  std::thread t([l] { l->Loop(); });
  t.join();
#endif

  return 0;
}