#include <csignal>
#include <iostream>
#include <vector>

#include "coding.h"
#include "config.h"
#include "logger.h"
#include "raft_server.h"
#include "test_suite.h"
#include "timer.h"
#include "vraft_logger.h"

void SignalHandler(int signal) {
  vraft::Logger::ShutDown();
  exit(signal);
}

std::vector<vraft::RaftServerPtr> raft_servers;
std::vector<vraft::Config> configs;

void Print(bool tiny, bool one_line) {
  for (auto ptr : raft_servers) {
    ptr->Print(tiny, one_line);
  }
  printf("\n");
  fflush(nullptr);
}

void TestTick2(vraft::Timer *timer) {
  switch (vraft::current_state) {
    case vraft::kTestState0: {
      Print(true, true);
      int32_t leader_num = 0;
      for (auto ptr : raft_servers) {
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
      exit(0);
    }
    default:
      break;
  }
}

extern void InitRemuTest();

int main(int argc, char **argv) {
  try {
    if (argc == 1) {
      std::cout << vraft::GetConfig().UsageBanner(argv[0]) << std::endl;
      return 0;
    }

    vraft::GetConfig().Init(argc, argv);
    if (vraft::GetConfig().result().count("h")) {
      std::cout << vraft::GetConfig().Usage() << std::endl;
      return 0;
    }

    vraft::LoggerOptions logger_options{"vraft", false, 1, 8192,
                                        vraft::kLoggerTrace};
    logger_options.level = vraft::U8ToLevel(vraft::GetConfig().log_level());
    logger_options.enable_debug = vraft::GetConfig().enable_debug();

    std::string log_file = vraft::GetConfig().path() + "/log/vraft.log";
    vraft::vraft_logger.Init(log_file, logger_options);

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGSEGV, SignalHandler);
    vraft::CodingInit();

    vraft::EventLoop loop("vraft_server");
    loop.AddTimer(1000, 1000, TestTick2);

    for (size_t i = 0; i < vraft::GetConfig().peers().size() + 1; ++i) {
      GenerateRotateConfig(configs);
      vraft::RaftServerPtr ptr =
          std::make_shared<vraft::RaftServer>(configs[i], &loop);
      raft_servers.push_back(ptr);
    }

    for (auto &ptr : raft_servers) {
      ptr->Start();
    }
    loop.Loop();

  } catch (const char *msg) {
    std::cerr << "execption caught: " << msg << std::endl;
  }

  return 0;
}
