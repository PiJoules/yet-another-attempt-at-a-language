#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ArgParser.h"
#include "Parser.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#define xstr(s) str(s)
#define str(s) #s
#define ASSERT(COND)                                                \
  {                                                                 \
    if (!(COND)) {                                                  \
      fputs("Assertion failed with '" xstr(COND) "' on line " xstr( \
                __LINE__) "\n",                                     \
            stderr);                                                \
      abort();                                                      \
    }                                                               \
  }

static llvm::LLVMContext GlobalContext;

namespace lang {
namespace ast {

class ASTVisitor {
 public:
  ASTVisitor(const std::string &ModuleID)
      : Module_(ModuleID, GlobalContext), Builder_(GlobalContext) {
    Module_.setTargetTriple(llvm::sys::getDefaultTargetTriple());
    PrintfFunc_ = CreatePrintfFunc();
  }

  void VisitModule(const Module &Mod) {
    for (const auto &ExternDecl : Mod.ExternDecls()) {
      // TODO: Other external decls
      VisitFunctionDecl(static_cast<const FunctionDeclaration &>(*ExternDecl));
    }
  }

  void VisitFunctionDecl(const FunctionDeclaration &FuncDecl) {
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
      if (const auto *exprstmt = dynamic_cast<const ExprStmt *>(stmt.get())) {
        VisitExprStmt(*exprstmt);
      } else if (const auto *retstmt =
                     dynamic_cast<const Return *>(stmt.get())) {
        VisitReturnStmt(*retstmt);
      } else {
        ASSERT(0 && "Unknown stmt type");
      }
    }
  }

  void VisitExprStmt(const ExprStmt &exprstmt) {
    VisitExpr(*exprstmt.Expression());
  }

  void VisitReturnStmt(const Return &retstmt) {
    Builder_.CreateRet(VisitExpr(*retstmt.Value()));
  }

  llvm::Value *VisitExpr(const Expr &expr) {
    if (const auto *call = dynamic_cast<const Call *>(&expr)) {
      return VisitCall(*call);
    } else if (const auto *str = dynamic_cast<const StringLiteral *>(&expr)) {
      return VisitStringLiteral(*str);
    } else if (const auto *intexpr =
                   dynamic_cast<const IntegerLiteral *>(&expr)) {
      return VisitIntExpr(*intexpr);
    } else if (const auto *id = dynamic_cast<const ID *>(&expr)) {
      return VisitID(*id);
    } else {
      ASSERT(0 && "Unhandled expression");
    }
  }

  llvm::Value *VisitID(const ID &id) {
    const std::string Name = id.Name();
    if (Name == "printf") {
      return PrintfFunc_;
    } else {
      ASSERT(0 && "Unknown variable");
    }
  }

  llvm::Value *VisitCall(const Call &call) {
    std::vector<llvm::Value *> Args;
    for (const auto &Arg : call.Args()) {
      Args.push_back(VisitExpr(*Arg));
    }
    return Builder_.CreateCall(VisitExpr(call.Caller()), Args);
  }

  llvm::Value *VisitStringLiteral(const StringLiteral &str) {
    return Builder_.CreateGlobalStringPtr(str.EscapedValue());
  }

  llvm::Value *VisitIntExpr(const IntegerLiteral &intexpr) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(GlobalContext),
                                  intexpr.Value());
  }

  llvm::Type *CreateType(const Type &Ty) {
    // TODO: Other types
    const auto &type = dynamic_cast<const Typename &>(Ty);
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

}  // namespace ast
}  // namespace lang

int main(int argc, char **argv) {
  lang::ArgParser parser;
  parser.AddArgument<lang::StringParsingMethod>("src", /*positional=*/true);
  parser.AddArgument<lang::StringParsingMethod>("--output");

  lang::ParsedArgs parsed_args = parser.Parse(argc, argv);
  if (!parser.DebugOk()) {
    return 1;
  }

  std::string Filename =
      parsed_args.GetArg<lang::StringArgument>("--output", "output.o")
          .getValue();

  std::error_code EC;
  llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::F_None);
  if (EC) {
    std::cerr << "Could not open file: " << EC.message() << std::endl;
    return 1;
  }

  std::ifstream input(argv[1]);
  lang::Parser Parse(input);
  std::unique_ptr<lang::ast::Module> Mod = Parse.Parse();
  ASSERT(Parse.Ok());  // TODO: Error checking

  lang::ast::ASTVisitor Visitor("asdf");
  Visitor.VisitModule(*Mod);
  // Visitor.Module().print(llvm::errs(), nullptr);

  // Initialize the target registry etc.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(
      llvm::sys::getDefaultTargetTriple(), Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!Target) {
    std::cerr << "Cannot find target: " << Error << std::endl;
    return 1;
  }

  auto TargetTriple = llvm::sys::getDefaultTargetTriple();
  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>();
  auto TargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  llvm::legacy::PassManager pass;
  auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

  if (TargetMachine->addPassesToEmitFile(pass, dest, FileType)) {
    std::cerr << "TargetMachine can't emit a file of this type" << std::endl;
    return 1;
  }

  pass.run(Visitor.Module());
  dest.flush();

  return 0;
}
