#include "vdb_console.h"
#include "vraft_logger.h"

vectordb::VdbConsoleSPtr gptr;

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << std::endl;
  gptr->Stop();

  std::cout << "vectordb-console stop ..." << std::endl;
  vraft::Logger::ShutDown();
  exit(0);
}

int main(int argc, char **argv) {
  vectordb::VdbConsoleSPtr console =
      std::make_shared<vectordb::VdbConsole>("vectordb", "127.0.0.1:9000");
  gptr = console;
  console->Run();

  return 0;
}