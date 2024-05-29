#ifndef VRAFT_ECHO_SERVER_H_
#define VRAFT_ECHO_SERVER_H_

#include <iostream>
#include <memory>

#include "config.h"
#include "eventloop.h"
#include "hostport.h"
#include "logger.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "vraft_logger.h"

class EchoServer;
using EchoServerPtr = std::shared_ptr<EchoServer>;

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
    std::cout << "echo-server recv " << s << std::endl;
    conn->CopySend(s.c_str(), s.size());
  }

  vraft::EventLoop loop_;
  vraft::TcpServer tcp_server_;
};

#endif