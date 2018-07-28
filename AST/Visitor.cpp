#include "ExternDecl.h"
#include "Visitor.h"

namespace lang {
namespace ast {

void Visitor::Visit(const Module &node) {
  for (const auto &extern_decls : node.ExternDecls())
    extern_decls->accept(*this);
}

void Visitor::Visit(const FunctionDeclaration &func_decl) {
  func_decl.ReturnType()->accept(*this);
  for (const auto &arg : func_decl.Args())
    arg->accept(*this);
  for (const auto &stmt : func_decl.Body())
    stmt->accept(*this);
}

void Visitor::Visit(const ArgumentDeclaration &arg_decl) {
  arg_decl.ArgType()->accept(*this);
}

void Visitor::Visit(const Return &ret) {
  ret.Value()->accept(*this);
}

void Visitor::Visit(const ExprStmt &exprstmt) {
  exprstmt.Expression()->accept(*this);
}

void Visitor::Visit(const Call &call) {
  call.Caller().accept(*this);
  for (const auto &arg : call.Args())
    arg->accept(*this);
}

void Visitor::Visit(const ID &id) {}
void Visitor::Visit(const StringLiteral &str) {}
void Visitor::Visit(const IntegerLiteral &integer) {}
void Visitor::Visit(const Typename &type) {}

}  // namespace ast
}  // namespace lang
