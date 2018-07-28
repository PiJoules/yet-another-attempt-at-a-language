#ifndef AST_ASTCOMMON_H_
#define AST_ASTCOMMON_H_

#include <ostream>

#include "Dump.h"
#include "Visitor.h"

#define ACCEPT_VISITORS \
  void accept(Visitor &visitor) const override { visitor.Visit(*this); }

namespace lang {
namespace ast {

class Node {
 public:
  virtual ~Node() {}

  virtual void accept(Visitor &visitor) const = 0;
};

}  // namespace ast
}  // namespace lang

#endif
