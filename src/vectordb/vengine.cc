#include "vengine.h"

#include "allocator.h"
#include "coding.h"
#include "common.h"
#include "vraft_logger.h"

namespace vectordb {

int32_t Vec::MaxBytes() {
  int32_t sz = 0;
  sz += sizeof(uint32_t);
  for (size_t i = 0; i < data.size(); ++i) {
    sz += sizeof(float);
  }
  return sz;
}

int32_t Vec::ToString(std::string &s) {
  s.clear();
  int32_t max_bytes = MaxBytes();
  char *ptr =
      reinterpret_cast<char *>(vraft::DefaultAllocator().Malloc(max_bytes));
  int32_t size = ToString(ptr, max_bytes);
  s.append(ptr, size);
  vraft::DefaultAllocator().Free(ptr);
  return size;
}

int32_t Vec::ToString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  uint32_t u32 = data.size();
  vraft::EncodeFixed32(p, u32);
  p += sizeof(u32);
  size += sizeof(u32);

  for (size_t i = 0; i < data.size(); ++i) {
    float f32 = data[i];
    vraft::EncodeFloat(p, f32);
    p += sizeof(f32);
    size += sizeof(f32);
  }

  assert(size <= len);
  return size;
}

int32_t Vec::FromString(std::string &s) {
  return FromString(s.c_str(), s.size());
}

int32_t Vec::FromString(const char *ptr, int32_t len) {
  char *p = const_cast<char *>(ptr);
  int32_t size = 0;

  uint32_t u32 = 0;
  u32 = vraft::DecodeFixed32(p);
  p += sizeof(u32);
  size += sizeof(u32);

  for (uint32_t i = 0; i < u32; ++i) {
    float f32 = vraft::DecodeFloat(p);
    p += sizeof(f32);
    size += sizeof(f32);
    data.push_back(f32);
  }

  return size;
}

nlohmann::json Vec::ToJson() {
  nlohmann::json j;
  for (size_t i = 0; i < data.size(); ++i) {
    float f32 = data[i];
    j[i] = f32;
  }
  return j;
}

nlohmann::json Vec::ToJsonTiny() {
  nlohmann::json j;
  for (size_t i = 0; i < data.size(); ++i) {
    float f32 = data[i];
    j[i] = f32;
  }
  return j;
}

std::string Vec::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["vec"] = ToJsonTiny();
  } else {
    j["vec"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

VEngine::VEngine(const std::string &path, int32_t dim)
    : path_(path),
      meta_path_(path + "/meta"),
      data_path_(path + "/data"),
      index_path_(path + "/index") {
  Init();
}

int32_t VEngine::Dim() const { return meta_->dim(); }

int32_t VEngine::Put(const std::string &key, const VecObj &vo) { return 0; }

int32_t VEngine::Get(const std::string &key, VecObj &vo) const { return 0; }

int32_t VEngine::Delete(const std::string &key) { return 0; }

int32_t VEngine::Load(const std::string &file_path) { return 0; }

bool VEngine::HasIndex() const { return 0; }

int32_t VEngine::AddIndex(VIndexType type) { return 0; }

int32_t VEngine::GetKNN(const std::string &key, int limit,
                        std::vector<VecResult> &results,
                        const std::string &index_name) {
  return 0;
}

int32_t VEngine::GetKNN(const std::vector<float> &vec, int limit,
                        std::vector<VecResult> &results,
                        const std::string &index_name) {
  return 0;
}

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
