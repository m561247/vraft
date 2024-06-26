#include "vdb_engine.h"

#include <cstdlib>
#include <fstream>
#include <iostream>

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
  std::ifstream file(file_path);
  if (!file.is_open()) {
    vraft::vraft_logger.FError("failed to open file: %s", file_path.c_str());
    return -1;
  }

  std::string line;
  int32_t line_num = 1;
  while (std::getline(file, line)) {
    std::cout << "load line " << line_num++ << ": " << line << std::endl;

    vraft::DelSpace(line);
    std::vector<std::string> result;
    vraft::Split(line, ';', result);

    if (result.size() != 3) {
      return -1;
    }

    std::string key = result[0];
    VecValue vv;
    std::string vec_str = result[1];
    std::vector<std::string> r;
    vraft::Split(vec_str, ',', r);
    for (auto f_str : r) {
      float f32;
      sscanf(f_str.c_str(), "%f", &f32);
      vv.vec.data.push_back(f32);
    }
    vv.attach_value = result[2];

    int32_t rv = Put(table, key, vv);
    if (rv != 0) {
      return rv;
    }
  }

  file.close();
  return 0;
}

bool VdbEngine::HasIndex(const std::string &table) const {
  bool b = true;
  meta_->ForEachReplicaInTable(table, [&b, this](ReplicaSPtr r) {
    auto it = this->engines_.find(r->uid);
    if (it == this->engines_.end()) {
      b = false;
    } else {
      if (it->second->HasIndex() == false) {
        b = false;
      }
    }
  });
  return b;
}

int32_t VdbEngine::AddIndex(const std::string &table, AddIndexParam param) {
  meta_->ForEachReplicaInTable(table, [this, param](ReplicaSPtr r) {
    VEngineSPtr ve;
    auto it = this->engines_.find(r->uid);
    if (it != this->engines_.end()) {
      ve = it->second;
      ve->AddIndex(param);
    }
  });

  return 0;
}

int32_t VdbEngine::GetKNN(const std::string &table, const std::string &key,
                          std::vector<VecResult> &results, int limit) {}

int32_t VdbEngine::GetKNN(const std::string &table,
                          const std::vector<float> &vec,
                          std::vector<VecResult> &results, int limit) {}

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
