#ifndef AST_EXTERNDECL_H_
#define AST_EXTERNDECL_H_

#include "ASTCommon.h"
#include "Stmt.h"
#include "Type.h"

namespace lang {
namespace ast {

class ExternalDeclaration : public Node {};

class ArgumentDeclaration : public Node {
 public:
  ArgumentDeclaration(std::unique_ptr<Type> Ty, std::string &Name)
      : Ty_(std::move(Ty)), Name_(Name) {}

  const Type *ArgType() const { return Ty_.get(); }
  std::string Name() const { return Name_; }

  ACCEPT_VISITORS;

 private:
  std::unique_ptr<Type> Ty_;
  std::string Name_;
};

class FunctionDeclaration : public ExternalDeclaration {
 public:
  FunctionDeclaration(std::unique_ptr<Type> RetType, const std::string &Name,
                      std::vector<std::unique_ptr<ArgumentDeclaration>> &Args,
                      std::vector<std::unique_ptr<Stmt>> &Body)
      : RetType_(std::move(RetType)),
        Name_(Name),
        Args_(std::move(Args)),
        Body_(std::move(Body)) {}

  const Type *ReturnType() const { return RetType_.get(); }
  std::string Name() const { return Name_; }
  const std::vector<std::unique_ptr<ArgumentDeclaration>> &Args() const {
    return Args_;
  }
  const std::vector<std::unique_ptr<Stmt>> &Body() const { return Body_; }

  ACCEPT_VISITORS;

 private:
  std::unique_ptr<Type> RetType_;
  std::string Name_;
  std::vector<std::unique_ptr<ArgumentDeclaration>> Args_;
  std::vector<std::unique_ptr<Stmt>> Body_;
};

class Module : public Node {
 public:
  explicit Module(
      std::vector<std::unique_ptr<ExternalDeclaration>> &ExternDecls)
      : ExternDecls_(std::move(ExternDecls)) {}

  const std::vector<std::unique_ptr<ExternalDeclaration>> &ExternDecls() const {
    return ExternDecls_;
  }

  ACCEPT_VISITORS;

 private:
  std::vector<std::unique_ptr<ExternalDeclaration>> ExternDecls_;
};

}  // namespace ast
}  // namespace lang

#endif
