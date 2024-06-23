#include "vindex_manager.h"

#include "vengine.h"

namespace vectordb {

VindexManager::VindexManager(VEngineSPtr v)
    : vengine_(v), path_(v->index_path()) {}

}  // namespace vectordb
