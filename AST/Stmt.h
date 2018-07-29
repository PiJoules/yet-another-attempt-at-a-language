#ifndef AST_STMT_H_
#define AST_STMT_H_

#include "ASTCommon.h"
#include "Expr.h"
#include "Type.h"

namespace lang {
namespace ast {

class Stmt : public Node {};

class ExprStmt : public Stmt {
 public:
  explicit ExprStmt(std::unique_ptr<Expr> E) : E_(std::move(E)) {}

  const Expr *Expression() const { return E_.get(); }

  ACCEPT_VISITORS;

 private:
  std::unique_ptr<Expr> E_;
};

class Return : public Stmt {
 public:
  Return(std::unique_ptr<Expr> RetVal) : RetVal_(std::move(RetVal)) {}

  const Expr *Value() const { return RetVal_.get(); }

  ACCEPT_VISITORS;

 private:
  std::unique_ptr<Expr> RetVal_;
};

class VarDecl : public Stmt {
 public:
  VarDecl(std::unique_ptr<Type> type, const std::string &varname,
          std::unique_ptr<Expr> init)
      : type_(std::move(type)), varname_(varname), init_(std::move(init)) {}
  VarDecl(std::unique_ptr<Type> type, const std::string &varname)
      : type_(std::move(type)), varname_(varname) {}

  std::string Name() const { return varname_; }
  const Expr &Init() const { return *init_; }
  const Type &VarType() const { return *type_; }
  bool HasInit() const { return init_ != nullptr; }

  ACCEPT_VISITORS;

 private:
  std::unique_ptr<Type> type_;
  std::string varname_;
  std::unique_ptr<Expr> init_;
};

}  // namespace ast
}  // namespace lang

#endif
