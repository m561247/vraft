#ifndef VRAFT_ECHO_SERVER_H_
#define VRAFT_ECHO_SERVER_H_

#include <csignal>
#include <iostream>

#include "clock.h"
#include "config.h"
#include "eventloop.h"
#include "hostport.h"
#include "tcp_client.h"
#include "tcp_connection.h"
#include "vraft_logger.h"

inline void Ping(vraft::TcpConnectionPtr &conn, vraft::Timer *t) {
  char buf[128];
  snprintf(buf, sizeof(buf), "ping %ld", vraft::Clock::Sec());
  conn->BufSend(buf, strlen(buf) + 1);
  vraft::vraft_logger.FInfo("echo-client send msg:[%s]", buf);
}

inline void OnConnection(const vraft::TcpConnectionPtr &conn) {
  vraft::vraft_logger.FInfo("echo-client OnConnection:%s",
                            conn->name().c_str());
  vraft::TimerParam param;
  param.timeout_ms = 0;
  param.repeat_ms = 1000;
  param.cb = std::bind(Ping, conn, std::placeholders::_1);
  param.data = nullptr;
  conn->loop()->AddTimer(param);
}

#endif