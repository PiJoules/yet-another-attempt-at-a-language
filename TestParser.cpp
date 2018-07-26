#include <fstream>

#include "Parser.h"
#include "gtest/gtest.h"

using lang::Parser;
using lang::ast::ArgumentDeclaration;
using lang::ast::Call;
using lang::ast::Expr;
using lang::ast::ExprStmt;
using lang::ast::FunctionDeclaration;
using lang::ast::ID;
using lang::ast::IntegerLiteral;
using lang::ast::Module;
using lang::ast::Node;
using lang::ast::Return;
using lang::ast::Stmt;
using lang::ast::StringLiteral;
using lang::ast::Type;
using lang::ast::Typename;

#define TEST_PARSER_ERR(CLASS, INPUT, TOK_KIND, PERR, PERR_NAME)   \
  TEST_F(ParserTest, Parse##CLASS##_##PERR_NAME) {                 \
    Input_ << INPUT;                                               \
    Parser Parse(Input_);                                          \
    std::unique_ptr<lang::ast::CLASS> node = Parse.Parse##CLASS(); \
    ASSERT_FALSE(Parse.Ok());                                      \
    ASSERT_EQ(node, nullptr);                                      \
    ASSERT_EQ(Parse.Status(), PERR);                               \
    ASSERT_EQ(Parse.LastReadTok().Kind, TOK_KIND);                 \
    ASSERT_STREQ(Parse.LastReadTok().Chars.c_str(), INPUT);        \
  }

#define TEST_UNEXPECTED_TOKEN(CLASS, INPUT, TOK_KIND)                       \
  TEST_PARSER_ERR(CLASS, INPUT, TOK_KIND, lang::PSTAT_UNEXPECTED_TOKEN_ERR, \
                  UnexpectedError)

#define TEST_PARSE_EXPR(CLASS, INPUT)                      \
  TEST_F(ParserTest, ParseExpr##CLASS) {                   \
    Input_ << INPUT;                                       \
    Parser Parse(Input_);                                  \
    std::unique_ptr<Expr> expr = Parse.ParseExpr();        \
    ASSERT_TRUE(Parse.Ok());                               \
    ASSERT_NE(expr, nullptr);                              \
    ASSERT_NE(dynamic_cast<CLASS *>(expr.get()), nullptr); \
  }

namespace {

class ParserTest : public ::testing::Test {
 protected:
  std::stringstream Input_;
};

TEST_F(ParserTest, ParseBadToken) {
  Input_ << "\n\"abcde\"" << static_cast<char>(128);
  Parser Parse(Input_);

  std::unique_ptr<StringLiteral> Str = Parse.ParseStringLiteral();
  ASSERT_TRUE(Parse.Ok());

  Str = Parse.ParseStringLiteral();
  ASSERT_FALSE(Parse.Ok());
  ASSERT_EQ(Str, nullptr);
  ASSERT_EQ(Parse.Status(), lang::PSTAT_LEXER_ERR);
  ASSERT_EQ(Parse.Lex().CharReadOnErr(), 128);
  ASSERT_EQ(Parse.Lex().Row(), 1);
  ASSERT_EQ(Parse.Lex().Col(), 7);
}

TEST_F(ParserTest, ParseStringLiteral) {
  Input_ << "\"abcde\"";
  Parser Parse(Input_);
  std::unique_ptr<StringLiteral> Str = Parse.ParseStringLiteral();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(Str, nullptr);
  ASSERT_STREQ(Str->Value().c_str(), "\"abcde\"");
}

TEST_UNEXPECTED_TOKEN(StringLiteral, "123", lang::TOK_INT)

TEST_F(ParserTest, ParseIntegerLiteral) {
  Input_ << "123";
  Parser Parse(Input_);
  std::unique_ptr<IntegerLiteral> Int = Parse.ParseIntegerLiteral();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(Int, nullptr);
  ASSERT_EQ(Int->Value(), 123);
}

TEST_UNEXPECTED_TOKEN(IntegerLiteral, "abc", lang::TOK_ID)
TEST_PARSER_ERR(IntegerLiteral, "36893488147419103232" /*2^65*/, lang::TOK_INT,
                lang::PSTAT_BAD_INT_ERR, BadInt);

TEST_F(ParserTest, ParseID) {
  Input_ << "abcde";
  Parser Parse(Input_);
  std::unique_ptr<Expr> id = Parse.ParseIDExpr();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(id, nullptr);
  ASSERT_NE(dynamic_cast<const ID *>(id.get()), nullptr);
  ASSERT_STREQ(dynamic_cast<const ID *>(id.get())->Name().c_str(), "abcde");
}

TEST_F(ParserTest, ParseCallNoArgs) {
  Input_ << "printf()";
  Parser Parse(Input_);
  std::unique_ptr<Expr> Func = Parse.ParseIDExpr();
  ASSERT_TRUE(Parse.Ok());

  const Call &call = static_cast<const Call &>(*Func);

  const ID &caller = static_cast<const ID &>(call.Caller());
  ASSERT_STREQ(caller.Name().c_str(), "printf");

  ASSERT_TRUE(call.Args().empty());
}

TEST_F(ParserTest, ParseCallOneArg) {
  Input_ << "printf(abc)";
  Parser Parse(Input_);
  std::unique_ptr<Expr> Func = Parse.ParseIDExpr();
  ASSERT_TRUE(Parse.Ok());

  const Call &call = static_cast<const Call &>(*Func);

  const ID caller = static_cast<const ID &>(call.Caller());
  ASSERT_STREQ(caller.Name().c_str(), "printf");

  const std::vector<std::unique_ptr<Expr>> &Args = call.Args();
  ASSERT_EQ(Args.size(), 1);

  const ID &Arg = static_cast<const ID &>(*Args.front());
  ASSERT_STREQ(Arg.Name().c_str(), "abc");
}

TEST_F(ParserTest, ParseCallMultipleArgs) {
  Input_ << "printf(abc, 123, \"str\")";
  Parser Parse(Input_);
  std::unique_ptr<Expr> Func = Parse.ParseIDExpr();
  ASSERT_TRUE(Parse.Ok());

  const Call &call = static_cast<const Call &>(*Func);

  const ID &caller = static_cast<const ID &>(call.Caller());
  ASSERT_STREQ(caller.Name().c_str(), "printf");

  const std::vector<std::unique_ptr<Expr>> &Args = call.Args();
  ASSERT_EQ(Args.size(), 3);

  const ID &Arg = static_cast<const ID &>(*Args[0]);
  ASSERT_STREQ(Arg.Name().c_str(), "abc");

  const IntegerLiteral &Arg2 = static_cast<const IntegerLiteral &>(*Args[1]);
  ASSERT_EQ(Arg2.Value(), 123);

  const StringLiteral &Arg3 = static_cast<const StringLiteral &>(*Args[2]);
  ASSERT_STREQ(Arg3.Value().c_str(), "\"str\"");
}

TEST_F(ParserTest, ParseCallNested) {
  Input_ << "printf(abc, call(\"str\"), 123)";
  Parser Parse(Input_);
  std::unique_ptr<Expr> Func = Parse.ParseIDExpr();
  ASSERT_TRUE(Parse.Ok());

  const Call &call = static_cast<const Call &>(*Func);

  const ID &caller = static_cast<const ID &>(call.Caller());
  ASSERT_STREQ(caller.Name().c_str(), "printf");

  const std::vector<std::unique_ptr<Expr>> &Args = call.Args();
  ASSERT_EQ(Args.size(), 3);

  const ID &Arg = static_cast<const ID &>(*Args[0]);
  ASSERT_STREQ(Arg.Name().c_str(), "abc");

  const Call &Arg2 = static_cast<const Call &>(*Args[1]);

  const std::vector<std::unique_ptr<Expr>> &NestedArgs = Arg2.Args();
  ASSERT_EQ(NestedArgs.size(), 1);

  const ID &NestedCaller = static_cast<const ID &>(Arg2.Caller());
  ASSERT_STREQ(NestedCaller.Name().c_str(), "call");

  const StringLiteral &NestedArg =
      static_cast<const StringLiteral &>(*NestedArgs[0]);
  ASSERT_STREQ(NestedArg.Value().c_str(), "\"str\"");

  const IntegerLiteral &Arg3 = static_cast<const IntegerLiteral &>(*Args[2]);
  ASSERT_EQ(Arg3.Value(), 123);
}

TEST_F(ParserTest, ParseExprStmt) {
  Input_ << "printf(\"str\");";
  Parser Parse(Input_);
  std::unique_ptr<Stmt> Stmt = Parse.ParseStmt();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(Stmt, nullptr);

  const auto *EStmt = static_cast<const ExprStmt *>(Stmt.get());
  ASSERT_NE(EStmt, nullptr);

  const Call *Func = static_cast<const Call *>(EStmt->Expression());
  ASSERT_NE(Func, nullptr);
  ASSERT_EQ(Func->Args().size(), 1);

  const ID &Caller = static_cast<const ID &>(Func->Caller());
  ASSERT_STREQ(Caller.Name().c_str(), "printf");

  const StringLiteral &Arg =
      static_cast<const StringLiteral &>(*(Func->Args()[0]));
  ASSERT_STREQ(Arg.Value().c_str(), "\"str\"");

  ASSERT_TRUE(Parse.ReachedEOF());
}

TEST_F(ParserTest, ParseReturn) {
  Input_ << "return 0;";
  Parser Parse(Input_);
  std::unique_ptr<Stmt> Stmt = Parse.ParseStmt();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(Stmt, nullptr);

  const auto *Ret = static_cast<const Return *>(Stmt.get());
  ASSERT_NE(Ret, nullptr);

  const auto *RetVal = dynamic_cast<const IntegerLiteral *>(Ret->Value());
  ASSERT_NE(RetVal, nullptr);
  ASSERT_EQ(RetVal->Value(), 0);

  ASSERT_TRUE(Parse.ReachedEOF());
}

TEST_F(ParserTest, Typename) {
  Input_ << "int";
  Parser Parse(Input_);
  std::unique_ptr<Type> Ty = Parse.ParseType();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(Ty, nullptr);

  const auto *Tyname = static_cast<const Typename *>(Ty.get());
  ASSERT_STREQ(Tyname->Name().c_str(), "int");
}

TEST_F(ParserTest, ArgDecl) {
  Input_ << "int x";
  Parser Parse(Input_);
  std::unique_ptr<ArgumentDeclaration> ArgDecl =
      Parse.ParseArgumentDeclaration();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(ArgDecl, nullptr);

  ASSERT_STREQ(ArgDecl->Name().c_str(), "x");

  const auto *Ty = static_cast<const Typename *>(ArgDecl->ArgType());
  ASSERT_NE(Ty, nullptr);
  ASSERT_STREQ(Ty->Name().c_str(), "int");
}

TEST_F(ParserTest, ParseFuncDecl) {
  Input_ << "int main() {\n"
            "  printf(\"str\");\n"
            "  return 0;\n"
            "}";
  Parser Parse(Input_);
  std::unique_ptr<FunctionDeclaration> FuncDecl =
      Parse.ParseFunctionDeclaration();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(FuncDecl, nullptr);

  // Return type
  const auto *RetTy = static_cast<const Typename *>(FuncDecl->ReturnType());
  ASSERT_NE(RetTy, nullptr);
  ASSERT_STREQ(RetTy->Name().c_str(), "int");

  // Name
  ASSERT_STREQ(FuncDecl->Name().c_str(), "main");

  // Arguments
  const auto &Args = FuncDecl->Args();
  ASSERT_EQ(Args.size(), 0);

  // Body
  const auto &Body = FuncDecl->Body();
  ASSERT_EQ(Body.size(), 2);
  const auto &Stmt1 = static_cast<const ExprStmt &>(*Body[0]);

  // printf
  const auto *call = static_cast<const Call *>(Stmt1.Expression());
  ASSERT_NE(call, nullptr);
  const auto &func = static_cast<const ID &>(call->Caller());
  ASSERT_STREQ(func.Name().c_str(), "printf");

  const auto &Stmt2 = static_cast<const Return &>(*Body[1]);
  const auto *retval = dynamic_cast<const IntegerLiteral *>(Stmt2.Value());
  ASSERT_NE(retval, nullptr);
  ASSERT_EQ(retval->Value(), 0);
}

TEST_F(ParserTest, FuncDeclArgsNoBody) {
  Input_ << "void func(int a, char b) {}";
  Parser Parse(Input_);
  std::unique_ptr<FunctionDeclaration> FuncDecl =
      Parse.ParseFunctionDeclaration();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(FuncDecl, nullptr);

  // Arguments
  const auto &Args = FuncDecl->Args();
  ASSERT_EQ(Args.size(), 2);

  const auto &Arg1 = static_cast<const ArgumentDeclaration &>(*Args[0]);
  const auto *Arg1Ty = static_cast<const Typename *>(Arg1.ArgType());
  ASSERT_NE(Arg1Ty, nullptr);
  ASSERT_STREQ(Arg1Ty->Name().c_str(), "int");
  ASSERT_STREQ(Arg1.Name().c_str(), "a");

  const auto &Arg2 = static_cast<const ArgumentDeclaration &>(*Args[1]);
  const auto *Arg2Ty = static_cast<const Typename *>(Arg2.ArgType());
  ASSERT_NE(Arg2Ty, nullptr);
  ASSERT_STREQ(Arg2Ty->Name().c_str(), "char");
  ASSERT_STREQ(Arg2.Name().c_str(), "b");

  // Body
  const auto &Body = FuncDecl->Body();
  ASSERT_EQ(Body.size(), 0);
}

TEST_PARSE_EXPR(IntegerLiteral, "123");
TEST_PARSE_EXPR(StringLiteral, "\"ab cd\"");

TEST_F(ParserTest, Empty) {
  Parser Parse(Input_);
  std::unique_ptr<Module> Mod = Parse.Parse();
  ASSERT_NE(Mod, nullptr);
  ASSERT_TRUE(Mod->ExternDecls().empty());
}

TEST_F(ParserTest, HelloWorld) {
  Input_ << "int main() {"
            "  printf(\"hello world\\n\");"
            "  return 0;"
            "}";
  Parser Parse(Input_);
  std::unique_ptr<Module> Mod = Parse.Parse();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(Mod, nullptr);

  ASSERT_EQ(Mod->ExternDecls().size(), 1);
  const auto &FuncDecl =
      static_cast<const FunctionDeclaration &>(*(Mod->ExternDecls()[0]));

  // Return type
  const auto *RetTy = static_cast<const Typename *>(FuncDecl.ReturnType());
  ASSERT_STREQ(RetTy->Name().c_str(), "int");

  // Name
  ASSERT_STREQ(FuncDecl.Name().c_str(), "main");

  // Arguments
  const auto &Args = FuncDecl.Args();
  ASSERT_EQ(Args.size(), 0);

  // Body
  const auto &Body = FuncDecl.Body();
  ASSERT_EQ(Body.size(), 2);
  const auto &Stmt1 = static_cast<const ExprStmt &>(*Body[0]);

  // printf
  const auto *call = dynamic_cast<const Call *>(Stmt1.Expression());
  ASSERT_NE(call, nullptr);
  const auto &func = static_cast<const ID &>(call->Caller());
  ASSERT_STREQ(func.Name().c_str(), "printf");

  const auto &Stmt2 = static_cast<const Return &>(*Body[1]);
  const auto *retval = dynamic_cast<const IntegerLiteral *>(Stmt2.Value());
  ASSERT_NE(retval, nullptr);
  ASSERT_EQ(retval->Value(), 0);
}

TEST_F(ParserTest, HelloWorldFromFile) {
  std::ifstream input("examples/hello_world.lang");
  Parser Parse(input);
  std::unique_ptr<Module> Mod = Parse.Parse();
  ASSERT_TRUE(Parse.Ok());
  ASSERT_NE(Mod, nullptr);

  ASSERT_EQ(Mod->ExternDecls().size(), 1);
  const auto &FuncDecl =
      static_cast<const FunctionDeclaration &>(*(Mod->ExternDecls()[0]));

  // Return type
  const auto *RetTy = static_cast<const Typename *>(FuncDecl.ReturnType());
  ASSERT_STREQ(RetTy->Name().c_str(), "int");

  // Name
  ASSERT_STREQ(FuncDecl.Name().c_str(), "main");

  // Arguments
  const auto &Args = FuncDecl.Args();
  ASSERT_EQ(Args.size(), 0);

  // Body
  const auto &Body = FuncDecl.Body();
  ASSERT_EQ(Body.size(), 2);
  const auto &Stmt1 = static_cast<const ExprStmt &>(*Body[0]);

  // printf
  const auto *call = static_cast<const Call *>(Stmt1.Expression());
  ASSERT_NE(call, nullptr);
  const auto &func = static_cast<const ID &>(call->Caller());
  ASSERT_STREQ(func.Name().c_str(), "printf");

  const auto &Stmt2 = static_cast<const Return &>(*Body[1]);
  const auto *retval = dynamic_cast<const IntegerLiteral *>(Stmt2.Value());
  ASSERT_NE(retval, nullptr);
  ASSERT_EQ(retval->Value(), 0);
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
