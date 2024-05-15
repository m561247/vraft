#include "clock.h"
#include "config.h"
#include "eventloop.h"
#include "hostport.h"
#include "tcp_client.h"
#include "tcp_connection.h"
#include "vraft_logger.h"
#include <csignal>
#include <iostream>

void Ping(vraft::TcpConnectionPtr &conn, vraft::Timer *t) {
  char buf[128];
  snprintf(buf, sizeof(buf), "ping %ld", vraft::Clock::Sec());
  conn->BufSend(buf, strlen(buf) + 1);
  vraft::vraft_logger.FInfo("echo-client send msg:[%s]", buf);
}

void OnConnection(const vraft::TcpConnectionPtr &conn) {
  vraft::vraft_logger.FInfo("echo-client OnConnection:%s",
                            conn->name().c_str());
  conn->loop()->AddTimer(0, 1000, std::bind(Ping, conn, std::placeholders::_1));
}

void SignalHandler(int signal) {
  vraft::Logger::ShutDown();
  exit(signal);
}

int main(int argc, char **argv) {

#if 0  
  if (argc == 1) {
    std::cout << vraft::GetConfig().UsageBanner(argv[0]) << std::endl;
    return 0;
  }
#endif

  vraft::GetConfig().Init(argc, argv);
  if (vraft::GetConfig().result().count("h")) {
    std::cout << vraft::GetConfig().Usage() << std::endl;
    return 0;
  }

  vraft::LoggerOptions logger_options{"vraft", false, 1, 8192,
                                      vraft::kLoggerTrace};
  // vraft::vraft_logger.Init(vraft::GetConfig().log_file(), logger_options);
  vraft::vraft_logger.Init("./log/echo_client.log", logger_options);

  std::signal(SIGINT, SignalHandler);
  std::signal(SIGSEGV, SignalHandler);

  vraft::TcpOptions opt = {true};
  // vraft::HostPort dest_addr(vraft::GetConfig().my_addr().ToString());
  vraft::HostPort dest_addr("127.0.0.1", 9988);

  vraft::EventLoop loop("echo_client_loop");
  vraft::TcpClient client("echo_client", &loop, dest_addr, opt);
  client.set_on_connection_cb(std::bind(&OnConnection, std::placeholders::_1));
  client.TimerConnect(100);
  loop.Loop();

  return 0;
}