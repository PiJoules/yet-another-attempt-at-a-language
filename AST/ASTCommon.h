#ifndef AST_ASTCOMMON_H_
#define AST_ASTCOMMON_H_

#include <ostream>

#include "Dump.h"

#define ACCEPT_VISITORS \
  void accept(ASTDumper &visitor) const override { visitor.Visit(*this); }

namespace lang {
namespace ast {

class Node {
 public:
  virtual ~Node() {}

  virtual void accept(ASTDumper &visitor) const = 0;
};

}  // namespace ast
}  // namespace lang

#endif
