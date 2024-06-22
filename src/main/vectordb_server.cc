#include <csignal>
#include <iostream>

#include "coding.h"
#include "vdb_config.h"
#include "vectordb.h"

vectordb::VectorDBSPtr dbptr;

void SignalHandler(int signal) {
  std::cout << "recv signal " << strsignal(signal) << ", quit ..." << std::endl;
  dbptr->Stop();
}

int main(int argc, char **argv) {
  std::signal(SIGINT, SignalHandler);
  vraft::CodingInit();
  vectordb::VdbConfigSPtr config = vectordb::ConfigSingleton::GetInstance();

  if (argc == 1) {
    std::cout << config->UsageBanner(argv[0]) << std::endl;
    return 0;
  }

  config->Parse(argc, argv);
  if (config->result().count("h")) {
    std::cout << config->Usage() << std::endl;
    return 0;
  }

  vectordb::VectorDBSPtr vdb;
  dbptr = vdb;
  vdb->Start();
  return 0;
}