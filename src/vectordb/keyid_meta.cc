#include "keyid_meta.h"

#include "vraft_logger.h"

namespace vectordb {

std::string EncodeAnnoyKey(std::string &key) {
  std::string k(ANNOY_KEY_PREFIX);
  k.append(key);
  return k;
}

vraft::Slice DecodeAnnoyKey(std::string &key) {
  size_t sz = key.size();
  size_t sz_prefix = std::string(ANNOY_KEY_PREFIX).size();
  assert(sz > sz_prefix);
  return vraft::Slice(key.c_str() + sz_prefix, sz - sz_prefix);
}

KeyidMeta::KeyidMeta(const std::string &path) : path_(path) { CreateDB(); }

int32_t KeyidMeta::Put(const std::string &key, uint32_t id) { return 0; }

int32_t KeyidMeta::Put(uint32_t id, const std::string &key) { return 0; }

int32_t KeyidMeta::Get(const std::string &key, uint32_t &id) { return 0; }

int32_t KeyidMeta::Get(uint32_t id, std::string &key) { return 0; }

int32_t KeyidMeta::CreateDB() {
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
  return 0;
}

}  // namespace vectordb
