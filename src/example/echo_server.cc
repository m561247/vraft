#include "config.h"
#include "eventloop.h"
#include "hostport.h"
#include "logger.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "vraft_logger.h"
#include <csignal>
#include <iostream>

class EchoServer {
public:
  EchoServer(const vraft::HostPort &listen_addr, vraft::TcpOptions &options)
      : loop_("echo_server_loop"),
        tcp_server_(listen_addr, "echo_server", options, &loop_) {
    tcp_server_.set_on_connection_cb(
        std::bind(&EchoServer::OnConnection, this, std::placeholders::_1));
    tcp_server_.set_on_message_cb(std::bind(&EchoServer::OnMessage, this,
                                            std::placeholders::_1,
                                            std::placeholders::_2));
  }

  void Start() {
    tcp_server_.Start();
    loop_.Loop();
  }

  void Stop() {
    tcp_server_.Stop();
    loop_.Stop();
  }

private:
  void OnConnection(const vraft::TcpConnectionPtr &conn) {
    vraft::vraft_logger.FInfo("echo-server OnConnection:%s",
                              conn->name().c_str());
  }

  void OnMessage(const vraft::TcpConnectionPtr &conn, vraft::Buffer *buf) {
    std::string s(buf->BeginRead(), buf->ReadableBytes());
    buf->RetrieveAll();
    vraft::vraft_logger.FInfo("echo-server OnMessage:[%s]", s.c_str());
    conn->CopySend(s.c_str(), s.size());
  }

  vraft::EventLoop loop_;
  vraft::TcpServer tcp_server_;
};

EchoServer *g_server = nullptr;

void SignalHandler(int signal) {
  if (g_server) {
    g_server->Stop();
  }
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
  vraft::vraft_logger.Init("./log/echo_server.log", logger_options);

  std::signal(SIGINT, SignalHandler);
  std::signal(SIGSEGV, SignalHandler);

  vraft::TcpOptions opt = {true};
  // vraft::HostPort listen_addr(vraft::GetConfig().my_addr().ToString());
  vraft::HostPort listen_addr("127.0.0.1", 9988);
  EchoServer server(listen_addr, opt);
  g_server = &server;

  vraft::vraft_logger.FInfo("%s", "echo-server start ...");
  server.Start();

  return 0;
}
