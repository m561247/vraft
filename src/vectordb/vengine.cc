#include "vengine.h"

#include "vraft_logger.h"

namespace vectordb {

VEngine::VEngine(const std::string &path, int32_t dim)
    : path_(path),
      meta_path_(path + "/meta"),
      data_path_(path + "/data"),
      index_path_(path + "/index") {
  Init();
}

int32_t VEngine::Dim() const { return meta_->dim(); }

void VEngine::Init() {
  // init data
  db_options_.create_if_missing = true;
  db_options_.error_if_exists = false;
  leveldb::DB *dbptr;
  leveldb::Status status = leveldb::DB::Open(db_options_, path_, &dbptr);
  if (!status.ok()) {
    vraft::vraft_logger.FError("leveldb open %s error, %s", path_.c_str(),
                               status.ToString().c_str());
    assert(0);
  }
  db_.reset(dbptr);

  // init meta
  meta_ = std::make_shared<EngineMeta>(meta_path_);
  assert(meta_);

  // init index
}

}  // namespace vectordb
