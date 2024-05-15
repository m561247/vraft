#ifndef VRAFT_TPL_H_
#define VRAFT_TPL_H_

namespace vraft {

class Tpl final {
public:
  Tpl();
  ~Tpl();
  Tpl(const Tpl &t) = delete;
  Tpl &operator=(const Tpl &t) = delete;

private:
};

inline Tpl::Tpl() {}

inline Tpl::~Tpl() {}

} // namespace vraft

#endif
