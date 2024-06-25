#ifndef VECTORDB_VDB_COMMON_H_
#define VECTORDB_VDB_COMMON_H_

#include <memory>

namespace vectordb {

#define DEFAULT_LIMIT 10
#define SEARCH_K 1000000

class Table;
using TableSPtr = std::shared_ptr<Table>;
using TableUPtr = std::unique_ptr<Table>;
using TableWPtr = std::weak_ptr<Table>;

class Partition;
using PartitionSPtr = std::shared_ptr<Partition>;
using PartitionUPtr = std::unique_ptr<Partition>;
using PartitionWPtr = std::weak_ptr<Partition>;

class Replica;
using ReplicaSPtr = std::shared_ptr<Replica>;
using ReplicaUPtr = std::unique_ptr<Replica>;
using ReplicaWPtr = std::weak_ptr<Replica>;

class Metadata;
using MetadataSPtr = std::shared_ptr<Metadata>;
using MetadataUPtr = std::unique_ptr<Metadata>;
using MetadataWPtr = std::weak_ptr<Metadata>;

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

class VindexAnnoy;
using VindexAnnoySPtr = std::shared_ptr<VindexAnnoy>;
using VindexAnnoyUPtr = std::unique_ptr<VindexAnnoy>;
using VindexAnnoyWPtr = std::weak_ptr<VindexAnnoy>;

class EngineMeta;
using EngineMetaSPtr = std::shared_ptr<EngineMeta>;
using EngineMetaUPtr = std::unique_ptr<EngineMeta>;
using EngineMetaWPtr = std::weak_ptr<EngineMeta>;

class KeyidMeta;
using KeyidMetaSPtr = std::shared_ptr<KeyidMeta>;
using KeyidMetaUPtr = std::unique_ptr<KeyidMeta>;
using KeyidMetaWPtr = std::weak_ptr<KeyidMeta>;

class VindexMeta;
using VindexMetaSPtr = std::shared_ptr<VindexMeta>;
using VindexMetaUPtr = std::unique_ptr<VindexMeta>;
using VindexMetaWPtr = std::weak_ptr<VindexMeta>;

}  // namespace vectordb

#endif
