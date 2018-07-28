#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "AST/ExternDecl.h"
#include "AST/Visitor.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

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
class CodeGen : public ast::Visitor {
 public:
  CodeGen(const std::string &ModuleID)
      : Module_(ModuleID, GlobalContext), Builder_(GlobalContext) {
    Module_.setTargetTriple(llvm::sys::getDefaultTargetTriple());
    PrintfFunc_ = CreatePrintfFunc();
  }

  void VisitModule(const ast::Module &Mod) {
    for (const auto &ExternDecl : Mod.ExternDecls()) {
      // TODO: Other external decls
      VisitFunctionDecl(
          static_cast<const ast::FunctionDeclaration &>(*ExternDecl));
    }
  }

  void VisitFunctionDecl(const ast::FunctionDeclaration &FuncDecl) {
    const std::string &FuncName = FuncDecl.Name();

    // TODO: Function arguments
    llvm::FunctionType *funcType =
        llvm::FunctionType::get(CreateType(*FuncDecl.ReturnType()), false);

    llvm::Function *func = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, FuncName, &Module_);
    auto *entry =
        llvm::BasicBlock::Create(GlobalContext, FuncName + "_func", func);
    Builder_.SetInsertPoint(entry);

    for (const auto &stmt : FuncDecl.Body()) {
      if (const auto *exprstmt =
              dynamic_cast<const ast::ExprStmt *>(stmt.get())) {
        VisitExprStmt(*exprstmt);
      } else if (const auto *retstmt =
                     dynamic_cast<const ast::Return *>(stmt.get())) {
        VisitReturnStmt(*retstmt);
      } else {
        ASSERT(0 && "Unknown stmt type");
      }
    }
  }

  void VisitExprStmt(const ast::ExprStmt &exprstmt) {
    VisitExpr(*exprstmt.Expression());
  }

  void VisitReturnStmt(const ast::Return &retstmt) {
    Builder_.CreateRet(VisitExpr(*retstmt.Value()));
  }

  llvm::Value *VisitExpr(const ast::Expr &expr) {
    if (const auto *call = dynamic_cast<const ast::Call *>(&expr)) {
      return VisitCall(*call);
    } else if (const auto *str =
                   dynamic_cast<const ast::StringLiteral *>(&expr)) {
      return VisitStringLiteral(*str);
    } else if (const auto *intexpr =
                   dynamic_cast<const ast::IntegerLiteral *>(&expr)) {
      return VisitIntExpr(*intexpr);
    } else if (const auto *id = dynamic_cast<const ast::ID *>(&expr)) {
      return VisitID(*id);
    } else {
      ASSERT(0 && "Unhandled expression");
    }
  }

  llvm::Value *VisitID(const ast::ID &id) {
    const std::string Name = id.Name();
    if (Name == "printf") {
      return PrintfFunc_;
    } else {
      ASSERT(0 && "Unknown variable");
    }
  }

  llvm::Value *VisitCall(const ast::Call &call) {
    std::vector<llvm::Value *> Args;
    for (const auto &Arg : call.Args()) {
      Args.push_back(VisitExpr(*Arg));
    }
    return Builder_.CreateCall(VisitExpr(call.Caller()), Args);
  }

  llvm::Value *VisitStringLiteral(const ast::StringLiteral &str) {
    return Builder_.CreateGlobalStringPtr(str.EscapedValue());
  }

  llvm::Value *VisitIntExpr(const ast::IntegerLiteral &intexpr) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(GlobalContext),
                                  intexpr.Value());
  }

  llvm::Type *CreateType(const ast::Type &Ty) {
    // TODO: Other types
    const auto &type = dynamic_cast<const ast::Typename &>(Ty);
    const std::string &Name = type.Name();
    if (Name == "int") {
      return Builder_.getInt32Ty();
    } else {
      ASSERT(0 && "Unknown typename");
    }
  }

  llvm::Module &Module() { return Module_; }

 private:
  llvm::Constant *CreatePrintfFunc() {
    std::vector<llvm::Type *> PrintfArgs;
    PrintfArgs.push_back(Builder_.getInt8Ty()->getPointerTo());
    llvm::ArrayRef<llvm::Type *> argsRef(PrintfArgs);
    llvm::FunctionType *PrintfType =
        llvm::FunctionType::get(Builder_.getInt32Ty(), argsRef,
                                /*isVarArg=*/true);
    return Module_.getOrInsertFunction("printf", PrintfType);
  }

  llvm::Module Module_;
  llvm::IRBuilder<> Builder_;

  llvm::Constant *PrintfFunc_;
};

}  // namespace lang

#endif
