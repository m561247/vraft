#ifndef VECTORDB_PARSER_H_
#define VECTORDB_PARSER_H_

namespace vectordb {

class Parser final {
public:
  explicit Parser();
  ~Parser();
  Parser(const Parser &t) = delete;
  Parser &operator=(const Parser &t) = delete;

private:
};

inline Parser::Parser() {}

inline Parser::~Parser() {}

} // namespace vectordb

#endif
