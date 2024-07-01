#include <csignal>
#include <iostream>

#include "coding.h"
#include "vraft_logger.h"
#include "vstore.h"
#include "vstore_common.h"

vstore::VstoreWPtr wptr;

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << ", quit ..." << std::endl;
  auto sptr = wptr.lock();
  if (sptr) {
    sptr->Stop();
  }
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

  vstore::VstoreSPtr vst = std::make_shared<vstore::Vstore>();
  wptr = vst;
  vst->Start();
  vst->Join();

  std::cout << "vstore-server stop ..." << std::endl;
  vraft::Logger::ShutDown();
  return 0;
}