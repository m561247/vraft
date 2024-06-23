#ifndef VECTORDB_VDB_COMMON_H_
#define VECTORDB_VDB_COMMON_H_

#include <memory>

namespace vectordb {

class Vindex;
using VindexSPtr = std::shared_ptr<Vindex>;
using VindexUPtr = std::unique_ptr<Vindex>;
using VindexWPtr = std::weak_ptr<Vindex>;

class VindexManager;
using VindexManagerSPtr = std::shared_ptr<VindexManager>;
using VindexManagerUPtr = std::unique_ptr<VindexManager>;
using VindexManagerWPtr = std::weak_ptr<VindexManager>;

class VEngine;
using VEngineSPtr = std::shared_ptr<VEngine>;
using VEngineUPtr = std::unique_ptr<VEngine>;
using VEngineWPtr = std::weak_ptr<VEngine>;

}  // namespace vectordb

#endif
