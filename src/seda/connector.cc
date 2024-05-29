#include "connector.h"

#include <cassert>

namespace vraft {

void ConnectFinish(UvConnect *req, int32_t status) {
  Connector *c = reinterpret_cast<Connector *>(req->data);
  c->loop_->AssertInLoopThread();

  if (status == 0) {
    if (c->new_conn_func_) {
      c->new_conn_func_(std::move(c->conn_));
    }

    vraft_logger.FInfo("connector:%s connect ok, retry_left:%ld, timer stop",
                       c->dest_addr().ToString().c_str(),
                       c->retry_timer_->repeat_counter());
    if (c->retry_timer_->IsStart()) {
      c->retry_timer_->Stop();
    }

  } else {
    vraft_logger.FError("connector:%s connect error",
                        c->dest_addr().ToString().c_str());
  }
}

void TimerConnectCb(Timer *timer) {
  Connector *c = reinterpret_cast<Connector *>(timer->data());
  if (timer->repeat_counter() > 0) {
    timer->RepeatDecr();
    int32_t r = c->Connect();
    if (r == 0) {
      // get result in ConnectFinish

    } else {
      vraft_logger.FError("connector:%s connect error, retry_left:%ld",
                          c->dest_addr().ToString().c_str(),
                          timer->repeat_counter());
    }

  } else {
    vraft_logger.FError(
        "connector:%s connect error, retry_left:%ld, timer stop",
        c->dest_addr().ToString().c_str(), timer->repeat_counter());
    timer->Stop();
  }
}

Connector::Connector(const HostPort &dest_addr, const TcpOptions &options,
                     EventLoop *loop)
    : dest_addr_(dest_addr), options_(options), loop_(loop) {
  Init();
  vraft_logger.FInfo("connector:%s construct, handle:%p, conn_req:%p",
                     dest_addr_.ToString().c_str(), conn_.get(), &connect_req_);
}

Connector::~Connector() {
  vraft_logger.FInfo("connector:%s destruct, handle:%p, conn_req:%p",
                     dest_addr_.ToString().c_str(), conn_.get(), &connect_req_);
}

int32_t Connector::TimerConnect(int64_t retry_times) {
  if (retry_times > 0) {
    retry_timer_->set_repeat_times(retry_times);
    loop_->AddTimer(retry_timer_);
  }
  return 0;
}

int32_t Connector::Connect(int64_t retry_times) {
  int32_t rv = 0;
  for (int64_t i = 0; i < retry_times; ++i) {
    rv = Connect();
    if (rv == 0) {
      break;
    }
  }
  return rv;
}

int32_t Connector::Connect() {
  loop_->AssertInLoopThread();
  return UvTcpConnect(&connect_req_, conn_.get(), &(dest_addr_.addr),
                      ConnectFinish);
}

int32_t Connector::Close() {
  if (conn_) {
    UvClose(reinterpret_cast<UvHandle *>(conn_.get()), nullptr);
  }
  retry_timer_->Close();
  return 0;
}

void Connector::Init() {
  conn_ = std::make_unique<UvTcp>();
  UvTcpInit(loop_->UvLoopPtr(), conn_.get());
  connect_req_.data = this;

  TimerParam param;
  param.timeout_ms = 0;
  param.repeat_ms = options_.retry_interval_ms;
  param.cb = TimerConnectCb;
  param.data = this;
  retry_timer_ = loop_->MakeTimer(param);
  assert(retry_timer_);
}

}  // namespace vraft