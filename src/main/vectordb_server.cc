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

  vectordb::VectorDBSPtr vdb;
  dbptr = vdb;
  vdb->Start();
  return 0;
}