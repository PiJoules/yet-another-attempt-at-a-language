#ifndef DUMP_H_
#define DUMP_H_

#include <iostream>

#include "Visitor.h"

namespace lang {
namespace ast {

class ArgumentDeclaration;
class Call;
class ExprStmt;
class FunctionDeclaration;
class ID;
class IntegerLiteral;
class Module;
class Return;
class StringLiteral;
class Typename;

class ASTDumper : public Visitor {
 public:
  ASTDumper(std::ostream &out) : out_(out) {}

  void Visit(const Module &module) override;
  void Visit(const FunctionDeclaration &func_decl) override;
  void Visit(const ArgumentDeclaration &arg_decl) override;
  void Visit(const Return &ret) override;
  void Visit(const ExprStmt &exprstmt) override;
  void Visit(const Call &call) override;
  void Visit(const ID &id) override;
  void Visit(const StringLiteral &str) override;
  void Visit(const IntegerLiteral &integer) override;
  void Visit(const Typename &type) override;

 private:
  void AddPadding() const {
    for (unsigned i = 0; i < level_; ++i) {
      out_ << "  ";
    }
  }

  std::ostream &out_;
  unsigned level_ = 0;
};

}  // namespace ast
}  // namespace lang

#endif
