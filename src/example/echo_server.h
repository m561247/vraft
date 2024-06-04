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
using EchoServerSPtr = std::shared_ptr<EchoServer>;
using EchoServerWPtr = std::weak_ptr<EchoServer>;

class EchoServer {
 public:
  EchoServer(const vraft::HostPort &listen_addr, vraft::TcpOptions &options) {
    loop_ = std::make_shared<vraft::EventLoop>("echo-loop");
    int32_t rv = loop_->Init();
    assert(rv == 0);

    tcp_server_ = std::make_shared<vraft::TcpServer>(loop_, listen_addr,
                                                     "echo-server", options);
    tcp_server_->set_on_connection_cb(
        std::bind(&EchoServer::OnConnection, this, std::placeholders::_1));
    tcp_server_->set_on_message_cb(std::bind(&EchoServer::OnMessage, this,
                                             std::placeholders::_1,
                                             std::placeholders::_2));
  }

  void Start() {
    tcp_server_->Start();
    loop_->Loop();
  }

  void Stop() {
    tcp_server_->Stop();
    loop_->Stop();
  }

 private:
  void OnConnection(const vraft::TcpConnectionSPtr &conn) {
    vraft::vraft_logger.FInfo("echo-server OnConnection, %s",
                              conn->DebugString().c_str());
  }

  void OnMessage(const vraft::TcpConnectionSPtr &conn, vraft::Buffer *buf) {
    std::string s(buf->BeginRead(), buf->ReadableBytes());
    buf->RetrieveAll();
    conn->CopySend(s.c_str(), s.size());
    s.push_back('\0');  // append '\0' for print
    vraft::vraft_logger.FInfo("echo-server OnMessage:[%s]", s.c_str());
    std::cout << "echo-server recv " << s << std::endl;
  }

  vraft::EventLoopSPtr loop_;
  vraft::TcpServerSPtr tcp_server_;
};

#endif