#ifndef VECTORDB_MESSAGE_H_
#define VECTORDB_MESSAGE_H_

namespace vectordb {

class Message final {
public:
  explicit Message();
  ~Message();
  Message(const Message &t) = delete;
  Message &operator=(const Message &t) = delete;

private:
};

inline Message::Message() {}

inline Message::~Message() {}

} // namespace vectordb

#endif
