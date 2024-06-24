#include "vindex_annoy.h"

#include "common.h"
#include "util.h"
#include "vengine.h"
#include "vindex_meta.h"

namespace vectordb {

VindexAnnoy::VindexAnnoy(VIndexParam &param, VEngineSPtr v)
    : Vindex(param),
      keyid_path_(param.path + "/keyid"),
      meta_path_(param.path + "/meta"),
      annoy_path_(param.path + "/annoy"),
      vengine_(v) {
  Init();
}

int32_t VindexAnnoy::GetKNN(const std::string &key, int limit,
                            std::vector<VecResult> &results) {
  return 0;
}

int32_t VindexAnnoy::GetKNN(const std::vector<float> &vec, int limit,
                            std::vector<VecResult> &results) {
  return 0;
}

nlohmann::json VindexAnnoy::ToJson() {
  nlohmann::json j;
  j["keyid_path"] = keyid_path_;
  j["annoy_path_"] = annoy_path_;
  j["meta_path_"] = meta_path_;
  j["meta"] = meta_->ToJson();
  return j;
}

nlohmann::json VindexAnnoy::ToJsonTiny() { return ToJson(); }

std::string VindexAnnoy::ToJsonString(bool tiny, bool one_line) {
  nlohmann::json j;
  if (tiny) {
    j["vindex-annoy"] = ToJsonTiny();
  } else {
    j["vindex-annoy"] = ToJson();
  }

  if (one_line) {
    return j.dump();
  } else {
    return j.dump(JSON_TAB);
  }
}

void VindexAnnoy::Init() {
  bool need_init = false;
  if (!vraft::IsDirExist(param().path)) {
    need_init = true;
  }

  if (need_init) {
    MkDir();
  }

  VIndexParam tmp_param = param();
  meta_ = std::make_shared<VindexMeta>(meta_path_, tmp_param);
  assert(meta_);
}

void VindexAnnoy::MkDir() {
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "mkdir -p %s", param().path.c_str());
  system(cmd);
}

}  // namespace vectordb
