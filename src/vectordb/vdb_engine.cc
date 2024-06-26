#include "vdb_engine.h"

#include <cstdlib>

#include "util.h"

namespace vectordb {

VdbEngine::VdbEngine(const std::string &path)
    : path_(path), meta_path_(path + "/meta"), data_path_(path + "/data") {
  Init();
}

int32_t VdbEngine::AddTable(AddTableParam param) {
  TableParam table_param;
  table_param.name = param.name;
  table_param.path = data_path_;
  table_param.partition_num = param.partition_num;
  table_param.replica_num = param.replica_num;
  table_param.dim = param.dim;

  int32_t rv = meta_->AddTable(table_param);
  if (rv != 0) {
    vraft::vraft_logger.FTrace("vdb-engine add table:%s error",
                               table_param.name);
    return -1;
  }

  meta_->ForEachReplicaInTable(param.name, [this](ReplicaSPtr replica) {
    int32_t rv = this->CreateVEngine(replica);
    assert(rv == 0);
  });

  return 0;
}

int32_t VdbEngine::Put(const std::string &table, const std::string &key,
                       VecValue &vv) {
  VEngineSPtr sptr = GetVEngine(table, key);
  if (!sptr) {
    return -1;
  }
  return sptr->Put(key, vv);
}

int32_t VdbEngine::Get(const std::string &table, const std::string &key,
                       VecObj &vo) {
  VEngineSPtr sptr = GetVEngine(table, key);
  if (!sptr) {
    return -1;
  }
  return sptr->Get(key, vo);
}

int32_t VdbEngine::Delete(const std::string &table, const std::string &key) {
  VEngineSPtr sptr = GetVEngine(table, key);
  if (!sptr) {
    return -1;
  }
  return sptr->Delete(key);
}

int32_t VdbEngine::Load(const std::string &table,
                        const std::string &file_path) {
  return 0;
}

nlohmann::json VdbEngine::ToJson() {
  nlohmann::json j;
  j["meta"] = meta_->ToJson();
  for (auto &kv : engines_) {
    j["engines"][kv.first] = kv.second->ToJson();
  }
  return j;
}

nlohmann::json VdbEngine::ToJsonTiny() { return ToJson(); }

std::string VdbEngine::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["vdb-engine"] = ToJsonTiny();
  } else {
    j["vdb-engine"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

int32_t VdbEngine::CreateVEngine(ReplicaSPtr replica) {
  vectordb::VEngineSPtr ve =
      std::make_shared<VEngine>(replica->path, replica->dim);
  engines_[replica->uid] = ve;
  return 0;
}

VEngineSPtr VdbEngine::GetVEngine(const std::string &table,
                                  const std::string &key) {
  VEngineSPtr sptr;
  TableSPtr table_sptr = meta_->GetTable(table);
  if (!table_sptr) {
    return sptr;
  }

  int32_t partition_id = vraft::PartitionId(key, table_sptr->partition_num);
  int32_t replica_id = 0;  // leader
  std::string replica_name = ReplicaName(table, partition_id, replica_id);
  ReplicaSPtr replica_sptr = meta_->GetReplica(replica_name);
  if (!replica_sptr) {
    return sptr;
  }

  auto it = engines_.find(replica_sptr->uid);
  if (it != engines_.end()) {
    sptr = it->second;
  }

  return sptr;
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
