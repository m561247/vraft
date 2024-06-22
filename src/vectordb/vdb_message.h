#ifndef VECTORDB_VDB_MESSAGE_H_
#define VECTORDB_VDB_MESSAGE_H_

namespace vectordb {

class VdbMessage final {
 public:
  explicit VdbMessage();
  ~VdbMessage();
  VdbMessage(const VdbMessage &) = delete;
  VdbMessage &operator=(const VdbMessage &) = delete;

 private:
};

inline VdbMessage::VdbMessage() {}

inline VdbMessage::~VdbMessage() {}

}  // namespace vectordb

#endif
