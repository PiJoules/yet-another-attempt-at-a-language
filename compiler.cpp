#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ArgParser.h"
#include "CodeGen.h"
#include "Parser.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

constexpr char SRC_FLAG[] = "src";
constexpr char OUTPUT_FLAG[] = "output";
constexpr char AST_DUMP_FLAG[] = "ast-dump";
constexpr char LLVM_DUMP_FLAG[] = "llvm-dump";

int main(int argc, char **argv) {
  lang::ArgParser parser;
  parser.AddPositionalArgument<lang::StringParsingMethod>(SRC_FLAG);

  struct lang::KWArgParams output_params = {};
  output_params.short_argname = 'o';
  parser.AddKeywordArgument<lang::StringParsingMethod>(OUTPUT_FLAG,
                                                       output_params);
  parser.AddEmptyKeywordArgument(AST_DUMP_FLAG);
  parser.AddEmptyKeywordArgument(LLVM_DUMP_FLAG);

  lang::ParsedArgs parsed_args = parser.Parse(argc, argv);
  if (!parser.DebugOk()) {
    return 1;
  }

  if (!parsed_args.HasArg(SRC_FLAG)) {
    std::cerr << "Expected 1 source file" << std::endl;
    return 1;
  }

  std::ifstream input(
      parsed_args.GetArg<lang::StringArgument>(SRC_FLAG).getValue());
  lang::Parser Parse(input);
  std::unique_ptr<lang::ast::Module> Mod = Parse.Parse();
  ASSERT(Parse.Ok());  // TODO: Error checking

  lang::CodeGen Generator("asdf");
  Generator.VisitModule(*Mod);
  if (parsed_args.HasArg(LLVM_DUMP_FLAG)) {
    Generator.Module().print(llvm::errs(), nullptr);
    return 0;
  } else if (parsed_args.HasArg(AST_DUMP_FLAG)) {
    lang::ast::ASTDumper dumper(std::cerr);
    dumper.Visit(*Mod);
    return 0;
  }

  std::string Filename =
      parsed_args.GetArg<lang::StringArgument>(OUTPUT_FLAG, "output.o")
          .getValue();

  std::error_code EC;
  llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::F_None);
  if (EC) {
    std::cerr << "Could not open file: " << EC.message() << std::endl;
    return 1;
  }

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

  pass.run(Generator.Module());
  dest.flush();

  return 0;
}
