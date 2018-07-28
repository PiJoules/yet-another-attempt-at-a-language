#include "Parser.h"
#include <memory>

using lang::ast::ArgumentDeclaration;
using lang::ast::Call;
using lang::ast::Expr;
using lang::ast::ExprStmt;
using lang::ast::ExternalDeclaration;
using lang::ast::FunctionDeclaration;
using lang::ast::ID;
using lang::ast::IntegerLiteral;
using lang::ast::Module;
using lang::ast::Return;
using lang::ast::Stmt;
using lang::ast::StringLiteral;
using lang::ast::Type;
using lang::ast::Typename;

namespace lang {

bool Parser::ReadAndCheckToken(enum TokenKind Expected) {
  if (!Lex_.ReadToken(LastReadTok_)) {
    Status_ = PSTAT_LEXER_ERR;
    return false;
  }
  if (LastReadTok_.Kind != Expected) {
    Status_ = PSTAT_UNEXPECTED_TOKEN_ERR;
    return false;
  }
  return true;
}

bool Parser::PeekAndCheckToken() {
  if (!Lex_.PeekToken(LastReadTok_)) {
    Status_ = PSTAT_LEXER_ERR;
    return false;
  }
  return true;
}

std::unique_ptr<Module> Parser::Parse() { return ParseModule(); }

/**
 * module ::= funcdecl*
 */
std::unique_ptr<Module> Parser::ParseModule() {
  ParserStack_.push_back("Module");
  std::vector<std::unique_ptr<ExternalDeclaration>> ExternDecls;

  while (Ok() && !ReachedEOF()) {
    std::unique_ptr<ExternalDeclaration> Decl = ParseFunctionDeclaration();
    if (!Decl) return nullptr;
    ExternDecls.push_back(std::move(Decl));
  }

  ParserStack_.pop_back();
  return std::make_unique<Module>(ExternDecls);
}

/**
 * type ::= id
 */
std::unique_ptr<Type> Parser::ParseType() {
  ParserStack_.push_back("Type");
  if (!ReadAndCheckToken(lang::TOK_ID)) return nullptr;
  ParserStack_.pop_back();
  return std::make_unique<Typename>(LastReadTok_.Chars);
}

/**
 * argdecl ::= type id
 */
std::unique_ptr<ArgumentDeclaration> Parser::ParseArgumentDeclaration() {
  ParserStack_.push_back("ArgumentDeclaration");
  std::unique_ptr<Type> Ty = ParseType();
  if (!Ty) return nullptr;

  if (!ReadAndCheckToken(lang::TOK_ID)) return nullptr;

  ParserStack_.pop_back();
  return std::make_unique<ArgumentDeclaration>(std::move(Ty),
                                               LastReadTok_.Chars);
}

/**
 * arglist ::= argdecl
 *         ::= argdecl (',' argdecl)*
 */
bool Parser::ParseArgList(
    std::vector<std::unique_ptr<ArgumentDeclaration>> &ArgList) {
  ParserStack_.push_back("ArgList");
  std::unique_ptr<ArgumentDeclaration> Arg = ParseArgumentDeclaration();
  if (!Arg) return false;
  ArgList.push_back(std::move(Arg));

  if (!PeekAndCheckToken()) return false;

  while (LastReadTok_.Kind == TOK_COMMA) {
    if (!(ReadAndCheckToken(lang::TOK_COMMA) &&
          (Arg = ParseArgumentDeclaration())))
      return false;

    ArgList.push_back(std::move(Arg));

    if (!PeekAndCheckToken()) return false;
  }
  ParserStack_.pop_back();
  return true;
}

/**
 * stmtlist ::= (stmt ';')+
 */
bool Parser::ParseStmtList(std::vector<std::unique_ptr<Stmt>> &StmtList) {
  ParserStack_.push_back("StmtList");
  std::unique_ptr<Stmt> stmt = ParseStmt();
  if (!stmt) return false;
  StmtList.push_back(std::move(stmt));

  if (!ReadAndCheckToken(lang::TOK_SEMICOL)) return false;

  if (!PeekAndCheckToken()) return false;

  while (LastReadTok_.Kind == TOK_SEMICOL) {
    if (!(ReadAndCheckToken(lang::TOK_SEMICOL) && (stmt = ParseStmt())))
      return false;

    StmtList.push_back(std::move(stmt));

    if (!PeekAndCheckToken()) return false;
  }
  ParserStack_.pop_back();
  return true;
}

/**
 * funcdecl ::= type id '(' ')' '{' stmtlist '}'
 * funcdecl ::= type id '(' arglist ')' '{' stmtlist '}'
 */
std::unique_ptr<FunctionDeclaration> Parser::ParseFunctionDeclaration() {
  ParserStack_.push_back("FunctionDeclaration");
  std::unique_ptr<Type> Ty = ParseType();
  if (!Ty) return nullptr;

  if (!ReadAndCheckToken(lang::TOK_ID)) return nullptr;

  std::string Name = LastReadTok_.Chars;

  if (!ReadAndCheckToken(lang::TOK_LPAR)) return nullptr;

  if (!PeekAndCheckToken()) return nullptr;

  std::vector<std::unique_ptr<ArgumentDeclaration>> ArgList;
  if (LastReadTok_.Kind != lang::TOK_RPAR && !ParseArgList(ArgList)) {
    Status_ = PSTAT_UNEXPECTED_TOKEN_ERR;
    return nullptr;
  }

  if (!ReadAndCheckToken(lang::TOK_RPAR) ||
      !ReadAndCheckToken(lang::TOK_LBRACE))
    return nullptr;

  if (!PeekAndCheckToken()) return nullptr;

  std::vector<std::unique_ptr<Stmt>> StmtList;
  while (LastReadTok_.Kind != lang::TOK_RBRACE) {
    std::unique_ptr<Stmt> S = ParseStmt();
    if (!S || !PeekAndCheckToken()) return nullptr;
    StmtList.push_back(std::move(S));
  }

  if (!ReadAndCheckToken(lang::TOK_RBRACE)) return nullptr;

  ParserStack_.pop_back();
  return std::make_unique<FunctionDeclaration>(std::move(Ty), Name, ArgList,
                                               StmtList);
}

/**
 * expr ::= int
 *      ::= str
 *      ::= idexpr
 */
std::unique_ptr<Expr> Parser::ParseExpr() {
  ParserStack_.push_back("Expr");
  if (!PeekAndCheckToken()) return nullptr;

  switch (LastReadTok_.Kind) {
    default:
      Status_ = PSTAT_UNEXPECTED_TOKEN_ERR;
      return nullptr;
    case TOK_INT:
      ParserStack_.pop_back();
      return ParseIntegerLiteral();
    case TOK_STR:
      ParserStack_.pop_back();
      return ParseStringLiteral();
    case TOK_ID:
      ParserStack_.pop_back();
      return ParseIDExpr();
  }
}

std::unique_ptr<StringLiteral> Parser::ParseStringLiteral() {
  ParserStack_.push_back("StringLiteral");
  if (!ReadAndCheckToken(lang::TOK_STR)) return nullptr;
  ParserStack_.pop_back();
  return std::make_unique<StringLiteral>(LastReadTok_.Chars);
}

std::unique_ptr<IntegerLiteral> Parser::ParseIntegerLiteral() {
  ParserStack_.push_back("IntegerLiteral");
  if (!ReadAndCheckToken(lang::TOK_INT)) return nullptr;
  auto Literal = IntegerLiteral::FromStr(LastReadTok_.Chars);
  if (!Literal) Status_ = PSTAT_BAD_INT_ERR;
  ParserStack_.pop_back();
  return Literal;
}

/**
 * exprlist ::= expr
 *          ::= expr (',' expr)*
 */
bool Parser::ParseExprList(std::vector<std::unique_ptr<Expr>> &ExprList) {
  ParserStack_.push_back("ExprList");
  std::unique_ptr<Expr> Arg = ParseExpr();
  if (!Arg) return false;
  ExprList.push_back(std::move(Arg));

  if (!PeekAndCheckToken()) return false;

  while (LastReadTok_.Kind == TOK_COMMA) {
    if (!(ReadAndCheckToken(lang::TOK_COMMA) && (Arg = ParseExpr())))
      return false;

    ExprList.push_back(std::move(Arg));

    if (!PeekAndCheckToken()) return false;
  }
  ParserStack_.pop_back();
  return true;
}

/**
 * idexpr ::= ID
 *        ::= idexpr '(' ')'
 *        ::= idexpr '(' exprlist ')'
 */
std::unique_ptr<Expr> Parser::ParseIDExpr() {
  ParserStack_.push_back("IDExpr");
  if (!ReadAndCheckToken(lang::TOK_ID)) return nullptr;
  Token Tok = LastReadTok_;
  auto Caller = std::make_unique<ID>(Tok.Chars);

  if (!PeekAndCheckToken()) return nullptr;

  if (LastReadTok_.Kind != TOK_LPAR) {
    ParserStack_.pop_back();
    return Caller;
  }

  if (!ReadAndCheckToken(lang::TOK_LPAR)) return nullptr;

  if (!PeekAndCheckToken()) return nullptr;

  if (LastReadTok_.Kind == lang::TOK_RPAR) {
    // Call with no args
    ParserStack_.pop_back();
    return std::make_unique<Call>(std::move(Caller));
  }

  std::vector<std::unique_ptr<Expr>> ExprList;
  if (!ParseExprList(ExprList)) {
    Status_ = PSTAT_UNEXPECTED_TOKEN_ERR;
    return nullptr;
  }

  if (!ReadAndCheckToken(lang::TOK_RPAR)) return nullptr;

  ParserStack_.pop_back();
  return std::make_unique<Call>(std::move(Caller), ExprList);
}

/**
 * stmt ::= 'return' expr ';'
 *      ::= expr ';'
 */
std::unique_ptr<Stmt> Parser::ParseStmt() {
  ParserStack_.push_back("Stmt");
  if (!PeekAndCheckToken()) return nullptr;

  std::unique_ptr<Stmt> stmt;
  switch (LastReadTok_.Kind) {
    case TOK_RETURN: {
      if (!ReadAndCheckToken(lang::TOK_RETURN)) return nullptr;
      std::unique_ptr<Expr> E = ParseExpr();
      if (!E) return nullptr;
      stmt.reset(new Return(std::move(E)));
      break;
    }
    default: {
      std::unique_ptr<Expr> E = ParseExpr();
      if (!E) return nullptr;
      stmt.reset(new ExprStmt(std::move(E)));
      break;
    }
  }

  if (!ReadAndCheckToken(lang::TOK_SEMICOL)) return nullptr;

  ParserStack_.pop_back();
  return stmt;
}

bool Parser::DebugOk() const {
  if (Status_ == PSTAT_OK) return true;

  switch (Status_) {
    case PSTAT_OK:
      return true;
    case PSTAT_LEXER_ERR:
      std::cerr << "Lexer error" << std::endl;
      break;
    case PSTAT_UNEXPECTED_TOKEN_ERR:
      std::cerr << "Unexpected token" << std::endl;
      LastReadTok().dump(std::cerr);
      std::cerr << std::endl;
      break;
    case PSTAT_BAD_INT_ERR:
      std::cerr << "Could not parse int literal" << std::endl;
      break;
  }

  DumpParseStack(std::cerr);
  return false;
}

}  // namespace lang
