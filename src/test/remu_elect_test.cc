#include <gtest/gtest.h>

#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include "coding.h"
#include "config.h"
#include "logger.h"
#include "raft_server.h"
#include "remu.h"
#include "test_suite.h"
#include "timer.h"
#include "vraft_logger.h"

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << std::endl;
}

vraft::EventLoopSPtr loop;
vraft::RemuSPtr remu;

void RemuTick(vraft::Timer *timer) {
  switch (vraft::current_state) {
    case vraft::kTestState0: {
      remu->Print();
      int32_t leader_num = 0;
      for (auto ptr : remu->raft_servers) {
        if (ptr->raft()->state() == vraft::LEADER) {
          leader_num++;
        }
      }

      if (leader_num == 1) {
        vraft::current_state = vraft::kTestStateEnd;
      }

      break;
    }
    case vraft::kTestStateEnd: {
      std::cout << "exit ..." << std::endl;
      remu->Stop();
      loop->Stop();
    }
    default:
      break;
  }
}

void GenerateConfig(std::vector<vraft::Config> &configs, int32_t peers_num) {
  vraft::GetConfig().set_my_addr(vraft::HostPort("127.0.0.1", 9000));
  for (int i = 1; i <= peers_num; ++i) {
    vraft::GetConfig().peers().push_back(
        vraft::HostPort("127.0.0.1", 9000 + i));
  }
  vraft::GetConfig().set_log_level(vraft::kLoggerTrace);
  vraft::GetConfig().set_enable_debug(true);
  vraft::GetConfig().set_path("/tmp/remu_test_dir");
  vraft::GetConfig().set_mode(vraft::kSingleMode);

  vraft::GenerateRotateConfig(configs);
}

class RemuTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::cout << "Setting up test... \n";
    std::fflush(nullptr);
    system("rm -rf /tmp/remu_test_dir");

    vraft::LoggerOptions logger_options{
        "vraft", false, 1, 8192, vraft::kLoggerTrace, true};
    std::string log_file = "/tmp/remu_test_dir/log/remu.log";
    vraft::vraft_logger.Init(log_file, logger_options);

    std::signal(SIGINT, SignalHandler);
    vraft::CodingInit();

    loop = std::make_shared<vraft::EventLoop>("remu-loop");
    int32_t rv = loop->Init();
    ASSERT_EQ(rv, 0);

    remu = std::make_shared<vraft::Remu>(loop);
  }

  void TearDown() override {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Tearing down test... \n";
    std::fflush(nullptr);

    // remu->Clear();
    remu.reset();
    loop.reset();
    vraft::Logger::ShutDown();

    // system("rm -rf /tmp/remu_test_dir");
  }
};

TEST_F(RemuTest, Elect3) {
  GenerateConfig(remu->configs, 2);
  remu->Create();
  remu->Start();
  vraft::TimerParam param;
  param.timeout_ms = 0;
  param.repeat_ms = 1000;
  param.cb = RemuTick;
  param.data = nullptr;
  loop->AddTimer(param);

  {
    vraft::EventLoopSPtr l = loop;
    std::thread t([l]() { l->Loop(); });
    l->WaitStarted();
    t.join();
  }

  std::cout << "join thread... \n";
  std::fflush(nullptr);
}

int main(int argc, char **argv) {
  vraft::CodingInit();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}