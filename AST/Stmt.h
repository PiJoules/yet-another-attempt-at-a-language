#ifndef AST_STMT_H_
#define AST_STMT_H_

#include "ASTCommon.h"
#include "Expr.h"

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

}  // namespace ast
}  // namespace lang

#endif
