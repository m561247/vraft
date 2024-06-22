#ifndef VECTORDB_CONFIG_H_
#define VECTORDB_CONFIG_H_

namespace vectordb {

class Config final {
public:
  explicit Config();
  ~Config();
  Config(const Config &t) = delete;
  Config &operator=(const Config &t) = delete;

private:
};

inline Config::Config() {}

inline Config::~Config() {}

} // namespace vectordb

#endif
