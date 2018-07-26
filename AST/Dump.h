#ifndef DUMP_H_
#define DUMP_H_

#include <iostream>

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

class ASTDumper {
 public:
  ASTDumper(std::ostream &out) : out_(out) {}

  void Visit(const Module &module);
  void Visit(const FunctionDeclaration &func_decl);
  void Visit(const ArgumentDeclaration &arg_decl);
  void Visit(const Return &ret);
  void Visit(const ExprStmt &exprstmt);
  void Visit(const Call &call);
  void Visit(const ID &id);
  void Visit(const StringLiteral &str);
  void Visit(const IntegerLiteral &integer);
  void Visit(const Typename &type);

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
