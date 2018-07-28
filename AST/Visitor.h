#ifndef AST_VISITOR_H_
#define AST_VISITOR_H_

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

class Visitor {
 public:
  virtual void Visit(const Module &module);
  virtual void Visit(const FunctionDeclaration &func_decl);
  virtual void Visit(const ArgumentDeclaration &arg_decl);
  virtual void Visit(const Return &ret);
  virtual void Visit(const ExprStmt &exprstmt);
  virtual void Visit(const Call &call);
  virtual void Visit(const ID &id);
  virtual void Visit(const StringLiteral &str);
  virtual void Visit(const IntegerLiteral &integer);
  virtual void Visit(const Typename &type);
};

}  // namespace ast
}  // namespace lang

#endif
