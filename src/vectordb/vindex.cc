#include "vindex.h"

#include <cassert>

#include "vindex_annoy.h"

namespace vectordb {

VindexSPtr VIndexFactory::Create(VIndexType index_type, const std::string &path,
                                 VEngineSPtr v, VIndexParam &param) {
  VindexSPtr index_sp;
  switch (index_type) {
    case kIndexAnnoy: {
      index_sp = std::make_shared<VindexAnnoy>(param, v);
      assert(index_sp);
      break;
    }

    default:
      assert(0);
  }

  return index_sp;
}

}  // namespace vectordb
