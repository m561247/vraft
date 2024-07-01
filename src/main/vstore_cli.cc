#include <csignal>
#include <iostream>

#include "coding.h"
#include "vraft_logger.h"
#include "vstore.h"
#include "vstore_common.h"

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << ", quit ..." << std::endl;
}

int main(int argc, char **argv) {
  std::signal(SIGINT, SignalHandler);
  vraft::CodingInit();

  vraft::LoggerOptions logger_options{
      "vstore", false, 1, 8192, vraft::kLoggerTrace, true};
  // logger_options.level = vraft::U8ToLevel(config->log_level());
  // logger_options.enable_debug = config->enable_debug();

  // std::string log_file = config->path() + "/log/vectordb-server.log";
  // vraft::vraft_logger.Init(log_file, logger_options);

  std::cout << "vstore-server stop ..." << std::endl;
  vraft::Logger::ShutDown();
  return 0;
}