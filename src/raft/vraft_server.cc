#include "coding.h"
#include "config.h"
#include "logger.h"
#include "raft_server.h"
#include "vraft_logger.h"
#include <csignal>
#include <iostream>

void SignalHandler(int signal) {
  vraft::Logger::ShutDown();
  exit(signal);
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
    vraft::vraft_logger.Init(vraft::GetConfig().log_file(), logger_options);

    std::signal(SIGINT, SignalHandler);
    std::signal(SIGSEGV, SignalHandler);
    vraft::CodingInit();

    if (vraft::GetConfig().mode() == vraft::kSingleMode) {
      vraft::RaftServer raft_server(vraft::GetConfig());
      raft_server.Start();

    } else if (vraft::GetConfig().mode() == vraft::kSingleMode) {
      assert(0);

    } else {
      assert(0);
    }

  } catch (const char *msg) {
    std::cerr << "execption caught: " << msg << std::endl;
  }

  return 0;
}
