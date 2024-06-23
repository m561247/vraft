#include <csignal>

#include "vdb_console.h"
#include "vraft_logger.h"

vectordb::VdbConsoleWPtr wptr;

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << std::endl;
  auto sptr = wptr.lock();
  if (sptr) {
    sptr->Stop();
  }

  std::cout << "vectordb-console stop ..." << std::endl;
  vraft::Logger::ShutDown();
  exit(0);
}

int main(int argc, char **argv) {
  std::signal(SIGINT, SignalHandler);
  vraft::CodingInit();

  vraft::LoggerOptions logger_options{"vectordb",          false, 1, 8192,
                                      vraft::kLoggerTrace, true};
  vraft::vraft_logger.Init("/tmp/vectordb-cli.log", logger_options);
  logger_options.enable_debug = true;

  vectordb::VdbConsoleSPtr console =
      std::make_shared<vectordb::VdbConsole>("vectordb-cli", "127.0.0.1:9000");
  wptr = console;
  console->Run();

  return 0;
}