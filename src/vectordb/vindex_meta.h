#ifndef VRAFT_VINDEX_META_H_
#define VRAFT_VINDEX_META_H_

namespace vraft {

class VindexMeta final {
public:
  explicit VindexMeta();
  ~VindexMeta();
  VindexMeta(const VindexMeta &) = delete;
  VindexMeta &operator=(const VindexMeta &) = delete;

private:
};

inline VindexMeta::VindexMeta() {}

inline VindexMeta::~VindexMeta() {}

} // namespace vraft

#endif
