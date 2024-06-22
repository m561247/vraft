#ifndef VECTORDB_TPL_H_
#define VECTORDB_TPL_H_

namespace vectordb {

class Tpl final {
public:
  explicit Tpl();
  ~Tpl();
  Tpl(const Tpl &) = delete;
  Tpl &operator=(const Tpl &) = delete;

private:
};

inline Tpl::Tpl() {}

inline Tpl::~Tpl() {}

} // namespace vectordb

#endif
