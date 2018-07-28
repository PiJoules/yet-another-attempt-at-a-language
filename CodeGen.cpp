#include "CodeGen.h"

namespace lang {

void CodeGen::Visit(const ast::Module &Mod) {
  for (const auto &extern_decl : Mod.ExternDecls()) extern_decl->accept(*this);
}

void CodeGen::Visit(const ast::FunctionDeclaration &FuncDecl) {
  const std::string &FuncName = FuncDecl.Name();

  // TODO: Function arguments
  llvm::FunctionType *funcType =
      llvm::FunctionType::get(CreateType(*FuncDecl.ReturnType()), false);

  llvm::Function *func = llvm::Function::Create(
      funcType, llvm::Function::ExternalLinkage, FuncName, &Module_);
  auto *entry =
      llvm::BasicBlock::Create(GlobalContext, FuncName + "_func", func);
  Builder_.SetInsertPoint(entry);

  for (const auto &stmt : FuncDecl.Body()) stmt->accept(*this);
}

void CodeGen::Visit(const ast::ExprStmt &exprstmt) {
  exprstmt.Expression()->accept(*this);
}

void CodeGen::Visit(const ast::Return &retstmt) {
  Builder_.CreateRet(CreateValue(*retstmt.Value()));
}

void CodeGen::Visit(const ast::ID &id) {
  const std::string Name = id.Name();
  if (Name == "printf") {
    SetReturnVal(PrintfFunc_);
  } else {
    ASSERT(0 && "Unknown variable");
  }
}

void CodeGen::Visit(const ast::Call &call) {
  std::vector<llvm::Value *> Args;
  for (const auto &Arg : call.Args()) {
    Args.push_back(CreateValue(*Arg));
  }
  SetReturnVal(Builder_.CreateCall(CreateValue(call.Caller()), Args));
}

void CodeGen::Visit(const ast::StringLiteral &str) {
  SetReturnVal(Builder_.CreateGlobalStringPtr(str.EscapedValue()));
}

void CodeGen::Visit(const ast::IntegerLiteral &intexpr) {
  SetReturnVal(llvm::ConstantInt::get(llvm::Type::getInt32Ty(GlobalContext),
                                      intexpr.Value()));
}

llvm::Type *CodeGen::CreateType(const ast::Type &Ty) {
  // TODO: Other types
  const auto &type = dynamic_cast<const ast::Typename &>(Ty);
  const std::string &Name = type.Name();
  if (Name == "int") {
    return Builder_.getInt32Ty();
  } else {
    ASSERT(0 && "Unknown typename");
  }
}

llvm::Constant *CodeGen::CreatePrintfFunc() {
  std::vector<llvm::Type *> PrintfArgs;
  PrintfArgs.push_back(Builder_.getInt8Ty()->getPointerTo());
  llvm::ArrayRef<llvm::Type *> argsRef(PrintfArgs);
  llvm::FunctionType *PrintfType =
      llvm::FunctionType::get(Builder_.getInt32Ty(), argsRef,
                              /*isVarArg=*/true);
  return Module_.getOrInsertFunction("printf", PrintfType);
}

}  // namespace lang
