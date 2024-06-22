#ifndef VECTORDB_VENGINE_H_
#define VECTORDB_VENGINE_H_

namespace vectordb {

class VEngine {
public:
  explicit VEngine();
  virtual ~VEngine();
  VEngine(const VEngine &) = delete;
  VEngine &operator=(const VEngine &) = delete;

private:
};

inline VEngine::VEngine() {}

inline VEngine::~VEngine() {}

} // namespace vectordb

#endif
