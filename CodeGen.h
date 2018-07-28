#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "AST/ExternDecl.h"
#include "AST/Visitor.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

// FIXME: assert() gets emitted when compiling on my env so use this temporary
// assert as a workaround.
#define xstr(s) stringify(s)
#define stringify(s) #s
#define ASSERT(COND)                                                \
  {                                                                 \
    if (!(COND)) {                                                  \
      fputs("Assertion failed with '" xstr(COND) "' on line " xstr( \
                __LINE__) "\n",                                     \
            stderr);                                                \
      abort();                                                      \
    }                                                               \
  }

// TODO: Move this out of a header
static llvm::LLVMContext GlobalContext;

namespace lang {

// TODO: Actually use the ast::Visitor methods
class CodeGen : public virtual ast::Visitor {
 public:
  CodeGen(const std::string &ModuleID)
      : Module_(ModuleID, GlobalContext), Builder_(GlobalContext) {
    Module_.setTargetTriple(llvm::sys::getDefaultTargetTriple());
    PrintfFunc_ = CreatePrintfFunc();
  }

  void Visit(const ast::Module &Mod) override;
  void Visit(const ast::FunctionDeclaration &FuncDecl) override;
  void Visit(const ast::ExprStmt &exprstmt) override;
  void Visit(const ast::Return &retstmt) override;

  void Visit(const ast::ID &id) override;
  void Visit(const ast::Call &call) override;
  void Visit(const ast::StringLiteral &str) override;
  void Visit(const ast::IntegerLiteral &intexpr) override;

  llvm::Type *CreateType(const ast::Type &Ty);

  llvm::Module &Module() { return Module_; }

 private:
  // TODO: Come up with a better way to return a new value on visiting an
  // expression for some visitors
  template <class ExprTy>
  llvm::Value *CreateValue(const ExprTy &E) {
    E.accept(*this);
    ASSERT(return_val_ &&
           "Expected visitor to call SetReturnVal() with a valid llvm::Value "
           "when visiting expression");
    llvm::Value *val = return_val_;
    return_val_ = nullptr;
    return val;
  }

  void SetReturnVal(llvm::Value *val) { return_val_ = val; }

  llvm::Constant *CreatePrintfFunc();

  llvm::Value *return_val_ = nullptr;

  llvm::Module Module_;
  llvm::IRBuilder<> Builder_;

  llvm::Constant *PrintfFunc_;
};

}  // namespace lang

#endif
