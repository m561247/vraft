#ifndef VECTORDB_VENGINE_H_
#define VECTORDB_VENGINE_H_

#include <memory>

#include "engine_meta.h"
#include "leveldb/db.h"
#include "nlohmann/json.hpp"
#include "vdb_common.h"
#include "vindex.h"

namespace vectordb {

struct Vec {
  std::vector<float> data;

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  int32_t FromString(std::string &s);
  int32_t FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

struct VecObj {
  std::string key;
  Vec vec;
  std::string attach_value;

  int32_t MaxBytes();
  int32_t ToString(std::string &s);
  int32_t ToString(const char *ptr, int32_t len);
  int32_t FromString(std::string &s);
  int32_t FromString(const char *ptr, int32_t len);

  nlohmann::json ToJson();
  nlohmann::json ToJsonTiny();
  std::string ToJsonString(bool tiny, bool one_line);
};

class VEngine final {
 public:
  explicit VEngine(const std::string &path, int32_t dim);
  ~VEngine();
  VEngine(const VEngine &) = delete;
  VEngine &operator=(const VEngine &) = delete;

  std::string path() const;
  std::string meta_path() const;
  std::string data_path() const;
  std::string index_path() const;
  int32_t Dim() const;

  int32_t Put(const std::string &key, const VecObj &vo);
  int32_t Get(const std::string &key, VecObj &vo) const;
  int32_t Delete(const std::string &key);
  int32_t Load(const std::string &file_path);

  bool HasIndex() const;
  int32_t AddIndex(VIndexType type);
  int32_t GetKNN(const std::string &key, int limit,
                 std::vector<VecResult> &results,
                 const std::string &index_name);
  int32_t GetKNN(const std::vector<float> &vec, int limit,
                 std::vector<VecResult> &results,
                 const std::string &index_name);

 private:
  void Init();

 private:
  std::string path_;
  std::string meta_path_;
  std::string data_path_;
  std::string index_path_;

  leveldb::Options db_options_;
  std::shared_ptr<leveldb::DB> db_;
  EngineMetaSPtr meta_;
  VindexManagerSPtr index_manager_;
};

inline VEngine::~VEngine() {}

inline std::string VEngine::path() const { return path_; }

inline std::string VEngine::meta_path() const { return meta_path_; }

inline std::string VEngine::data_path() const { return data_path_; }

inline std::string VEngine::index_path() const { return index_path_; }

}  // namespace vectordb

#endif
