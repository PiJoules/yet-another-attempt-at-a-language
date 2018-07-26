#include "Dump.h"
#include "ExternDecl.h"

namespace lang {
namespace ast {

void ASTDumper::Visit(const Module &module) {
  std::cerr << "init level " << level_ << std::endl;
  out_ << "Module\n";
  for (const auto &decl : module.ExternDecls()) {
    decl->accept(*this);
  }
}

void ASTDumper::Visit(const FunctionDeclaration &func_decl) {
  out_ << "|-FunctionDeclaration<\"" << func_decl.Name() << "\" -> ";
  func_decl.ReturnType()->accept(*this);
  out_ << ">(";
  for (const auto &arg : func_decl.Args()) {
    arg->accept(*this);
    out_ << "; ";
  }
  out_ << ")\n";

  level_++;

  for (auto const &stmt : func_decl.Body()) {
    stmt->accept(*this);
  }

  level_--;
}

void ASTDumper::Visit(const ArgumentDeclaration &arg_decl) {
  arg_decl.ArgType()->accept(*this);
  out_ << " " << arg_decl.Name();
}

void ASTDumper::Visit(const Return &ret) {
  AddPadding();
  out_ << "|-Return\n";
  level_++;
  ret.Value()->accept(*this);
  level_--;
}

void ASTDumper::Visit(const ExprStmt &exprstmt) {
  AddPadding();
  out_ << "|-ExprStmt\n";
  level_++;
  exprstmt.Expression()->accept(*this);
  level_--;
}

void ASTDumper::Visit(const Call &call) {
  AddPadding();
  out_ << "|-Call\n";
  level_++;

  AddPadding();
  out_ << "|-Caller\n";
  level_++;
  call.Caller().accept(*this);
  level_--;

  AddPadding();
  out_ << "|-Args\n";
  level_++;
  for (const auto &expr : call.Args()) {
    expr->accept(*this);
  }
  level_--;

  level_--;
}

void ASTDumper::Visit(const ID &id) {
  AddPadding();
  out_ << "|-ID<\"" << id.Name() << "\">\n";
}

void ASTDumper::Visit(const StringLiteral &str) {
  AddPadding();
  out_ << "|-StringLiteral<" << str.Value() << ">\n";
}

void ASTDumper::Visit(const IntegerLiteral &integer) {
  AddPadding();
  out_ << "|-IntegerLiteral<" << integer.Value() << ">\n";
}

void ASTDumper::Visit(const Typename &type) {
  out_ << "\"" << type.Name() << "\"";
}

}  // namespace ast
}  // namespace lang
