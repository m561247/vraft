#include "vdb_engine.h"

#include <cstdlib>

#include "util.h"

namespace vectordb {

VdbEngine::VdbEngine(const std::string &path)
    : path_(path), meta_path_(path + "/meta"), data_path_(path + "/data") {
  Init();
}

void VdbEngine::Init() {
  bool dir_exist = vraft::IsDirExist(path_);
  if (!dir_exist) {
    MkDir();
  }

  meta_ = std::make_shared<Metadata>(meta_path_);
  assert(meta_);
}

void VdbEngine::MkDir() {
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "mkdir -p %s", path_.c_str());
  system(cmd);
  snprintf(cmd, sizeof(cmd), "mkdir -p %s", data_path_.c_str());
  system(cmd);
}

}  // namespace vectordb
