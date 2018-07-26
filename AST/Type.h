#ifndef AST_TYPE_H_
#define AST_TYPE_H_

#include <string>

#include "ASTCommon.h"

namespace lang {
namespace ast {

class Type : public Node {};

class Typename : public Type {
 public:
  Typename(const std::string &Name) : Name_(Name) {}

  std::string Name() const { return Name_; }

  ACCEPT_VISITORS;

 private:
  std::string Name_;
};

}  // namespace ast
}  // namespace lang

#endif
