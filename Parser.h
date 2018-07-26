#ifndef PARSER_H_
#define PARSER_H_

#include <istream>
#include <memory>
#include <vector>

#include "AST/ASTCommon.h"
#include "AST/ExternDecl.h"
#include "Lexer.h"

namespace lang {

enum ParserStatus {
  PSTAT_OK,
  PSTAT_LEXER_ERR,
  PSTAT_UNEXPECTED_TOKEN_ERR,
  PSTAT_BAD_INT_ERR,
};

class Parser {
 public:
  explicit Parser(std::istream &Input) : Lex_(Input) {}

  std::unique_ptr<ast::Module> Parse();
  std::unique_ptr<ast::Module> ParseModule();
  std::unique_ptr<ast::FunctionDeclaration> ParseFunctionDeclaration();
  std::unique_ptr<ast::ArgumentDeclaration> ParseArgumentDeclaration();

  std::unique_ptr<ast::Expr> ParseExpr();
  std::unique_ptr<ast::StringLiteral> ParseStringLiteral();
  std::unique_ptr<ast::IntegerLiteral> ParseIntegerLiteral();
  std::unique_ptr<ast::Expr> ParseIDExpr();
  std::unique_ptr<ast::Type> ParseType();

  std::unique_ptr<ast::Stmt> ParseStmt();

  enum ParserStatus Status() const { return Status_; }
  bool Ok() const { return Status_ == PSTAT_OK; }
  const Lexer &Lex() const { return Lex_; }
  bool ReachedEOF() { return Lex_.ReachedEOF(); }

  /**
   * The last token read before an error is thrown.
   */
  Token LastReadTok() const { return LastReadTok_; }

  void DumpParseStack(std::ostream &out) const {
    out << "<";
    for (const std::string &rule : ParserStack_) {
      out << rule << "; ";
    }
    out << ">";
  }

 private:
  bool ParseExprList(std::vector<std::unique_ptr<ast::Expr>> &ExprList);
  bool ParseArgList(
      std::vector<std::unique_ptr<ast::ArgumentDeclaration>> &ArgList);
  bool ParseStmtList(std::vector<std::unique_ptr<ast::Stmt>> &StmtList);

  /**
   * Returns true if we can read a token off the lexer and matches the token
   * kind we expect. Returns false otherwise.
   *
   * If we cannot read a token off the lexer or this token does not match the
   * token we expect, we set the status appropriately.
   *
   * This method also updates the last token read.
   */
  bool ReadAndCheckToken(enum TokenKind Expected);
  bool PeekAndCheckToken();

  Lexer Lex_;
  enum ParserStatus Status_ = PSTAT_OK;
  Token LastReadTok_;
  std::vector<std::string> ParserStack_;
};

}  // namespace lang

#endif
