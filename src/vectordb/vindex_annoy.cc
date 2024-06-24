#include "vindex_annoy.h"

#include "annoylib.h"
#include "common.h"
#include "keyid_meta.h"
#include "kissrandom.h"
#include "util.h"
#include "vengine.h"
#include "vindex_meta.h"
#include "vraft_logger.h"

namespace vectordb {

AnnoySPtr CreateAnnoy(const VIndexParam &param) {
  AnnoySPtr sptr = nullptr;
  Annoy *ptr = nullptr;
  switch (param.distance_type) {
    case kCosine: {
      ptr = new AnnoyIndex<int32_t, float, Angular, Kiss32Random,
                           AnnoyIndexMultiThreadedBuildPolicy>(param.dim);
      sptr.reset(ptr);
      return sptr;
    }

    case kInnerProduct: {
      ptr = new AnnoyIndex<int32_t, float, DotProduct, Kiss32Random,
                           AnnoyIndexMultiThreadedBuildPolicy>(param.dim);
      sptr.reset(ptr);
      return sptr;
    }

    case kEuclidean: {
      ptr = new AnnoyIndex<int32_t, float, Euclidean, Kiss32Random,
                           AnnoyIndexMultiThreadedBuildPolicy>(param.dim);
      sptr.reset(ptr);
      return sptr;
    }

    default:
      return sptr;
  }
}

VindexAnnoy::VindexAnnoy(VIndexParam &param, VEngineSPtr v)
    : Vindex(param),
      keyid_path_(param.path + "/keyid"),
      meta_path_(param.path + "/meta"),
      annoy_path_file_(param.path + "/annoy.tree"),
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
  j["annoy_path_file"] = annoy_path_file_;
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

  keyid_ = std::make_shared<KeyidMeta>(keyid_path_);
  assert(keyid_);

  annoy_index_ = CreateAnnoy(param());
  assert(annoy_index_);

  int32_t rv = 0;
  if (need_init) {
    rv = Build();
    assert(rv == 0);
  } else {
    // rv = Load();
    assert(rv == 0);
  }
}

void VindexAnnoy::MkDir() {
  char cmd[128];
  snprintf(cmd, sizeof(cmd), "mkdir -p %s", param().path.c_str());
  system(cmd);
}

int32_t VindexAnnoy::Build() {
  VEngineSPtr ve = vengine_.lock();
  if (ve) {
    int32_t annoy_index_id = 0;
    leveldb::Iterator *it = ve->db()->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      std::string key = it->key().ToString();
      std::string value = it->value().ToString();

      VecValue vv;
      int32_t bytes = vv.FromString(value);
      assert(bytes > 0);

      if (vv.dim() != param().dim) {
        vraft::vraft_logger.FError("build index dim error, %d != %d", vv.dim(),
                                   param().dim);
        return -1;
      }

      std::vector<float> arr;
      for (int j = 0; j < vv.dim(); ++j) {
        arr.push_back(vv.vec.data[j]);
      }
      const float *parr = &(*arr.begin());
      annoy_index_->add_item(annoy_index_id, parr);

      int32_t rv = keyid_->Put(key, annoy_index_id);
      assert(rv == 0);

      rv = keyid_->Put(annoy_index_id, key);
      assert(rv == 0);

      ++annoy_index_id;
    }

    if (!it->status().ok()) {
      vraft::vraft_logger.FError("build index %s",
                                 it->status().ToString().c_str());
      return -1;
    }
    delete it;

    annoy_index_->build(param().annoy_tree_num);
    vraft::vraft_logger.FInfo("annoy build tree finish, path:%s", param().path);

    annoy_index_->save(annoy_path_file_.c_str());
    vraft::vraft_logger.FInfo("annoy save tree finish, path:%s", param().path);

    return 0;

  } else {
    return -1;
  }
}

}  // namespace vectordb
