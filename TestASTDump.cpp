#include <fstream>
#include <iostream>
#include <sstream>

#include "AST/Dump.h"
#include "AST/Expr.h"
#include "Parser.h"
#include "gtest/gtest.h"

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

using lang::Parser;
using lang::ast::ASTDumper;

namespace {

TEST(ASTTest, IntegerLiteral) {
  std::stringstream input;
  input << "2";
  ASTDumper dumper(input);
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
