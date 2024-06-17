#ifndef VRAFT_ECHO_SERVER_H_
#define VRAFT_ECHO_SERVER_H_

#include <iostream>
#include <memory>
#include <vector>

#include "config.h"
#include "eventloop.h"
#include "hostport.h"
#include "logger.h"
#include "server_thread.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "vraft_logger.h"

class EchoServer;
using EchoServerSPtr = std::shared_ptr<EchoServer>;
using EchoServerWPtr = std::weak_ptr<EchoServer>;

class EchoServer {
 public:
  EchoServer(vraft::HostPort listen_addr, vraft::TcpOptions &options,
             int32_t server_num)
      : server_thread_("echo-server-thread", false) {
    for (int32_t i = 0; i < server_num; ++i) {
      char name_buf[128];
      snprintf(name_buf, sizeof(name_buf), "echo-server-%d", i);

      auto sptr = server_thread_.LoopPtr();
      vraft::TcpServerSPtr tcp_server = std::make_shared<vraft::TcpServer>(
          sptr, name_buf,
          vraft::HostPort(listen_addr.host, listen_addr.port + i), options);
      tcp_server->set_on_connection_cb(
          std::bind(&EchoServer::OnConnection, this, std::placeholders::_1));
      tcp_server->set_on_message_cb(std::bind(&EchoServer::OnMessage, this,
                                              std::placeholders::_1,
                                              std::placeholders::_2));
      servers_.push_back(tcp_server);
      server_thread_.AddServer(tcp_server);
    }
  }

  void Start() { server_thread_.Start(); }
  void Join() { server_thread_.Join(); }
  void Stop() { server_thread_.Stop(); }

 private:
  void OnConnection(const vraft::TcpConnectionSPtr &conn) {
    vraft::vraft_logger.FInfo("echo-server OnConnection, %s",
                              conn->DebugString().c_str());
  }

  void OnMessage(const vraft::TcpConnectionSPtr &conn, vraft::Buffer *buf) {
    std::string s(buf->BeginRead(), buf->ReadableBytes());
    buf->RetrieveAll();
    conn->CopySend(s.c_str(), s.size());

    // delete 0xD 0xA, for print pretty
    if (s.size() >= 2) {
      if (s[s.length() - 1] == 0xA && s[s.length() - 2] == 0xD) {
        s.erase(s.size() - 2);
      }
    }

    vraft::vraft_logger.FInfo("connection:%s on-message:[%s]",
                              conn->name().c_str(), s.c_str());
    std::cout << "connection:" << conn->name() << " on-message: " << s
              << std::endl;
  }

  std::vector<vraft::TcpServerSPtr> servers_;
  vraft::ServerThread server_thread_;
};

#endif