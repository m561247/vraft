#include <csignal>
#include <iostream>
#include <vector>

#include "coding.h"
#include "config.h"
#include "logger.h"
#include "raft_server.h"
#include "vraft_logger.h"

void SignalHandler(int signal) {
  vraft::Logger::ShutDown();
  exit(signal);
}

#define SERVER_NUM 3
std::vector<vraft::RaftServerPtr> raft_servers;

void GenerateRotateConfig(std::vector<vraft::Config> &configs) {
  std::vector<vraft::HostPort> hps;
  hps.push_back(vraft::GetConfig().my_addr());
  for (auto hp : vraft::GetConfig().peers()) {
    hps.push_back(hp);
  }
  for (int32_t i = 0; i < hps.size(); ++i) {
    // use i for myself
    vraft::Config c = vraft::GetConfig();
    c.set_my_addr(hps[i]);
    c.peers().clear();
    for (int32_t j = 0; j < hps.size(); ++j) {
      if (j != i) {
        c.peers().push_back(hps[j]);
      }
    }
    std::string path = c.path();
    c.set_path(path + "/" + c.my_addr().ToString());
    configs.push_back(c);
  }
}

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
    for (int i = 0; i < vraft::GetConfig().peers().size() + 1; ++i) {
      std::vector<vraft::Config> configs;
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
