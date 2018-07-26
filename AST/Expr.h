#ifndef AST_EXPR_H_
#define AST_EXPR_H_

#include <memory>
#include <string>
#include <vector>

#include "ASTCommon.h"

namespace lang {
namespace ast {

class Expr : public Node {};

class IntegerLiteral : public Expr {
 public:
  IntegerLiteral(uint64_t Val) : Val_(Val) {}

  uint64_t Value() const { return Val_; }

  ACCEPT_VISITORS;

  /**
   * Attempts to parse an uint64_t from a string. Returns the integer literal
   * holding the value if successful and nullptr otherwise.
   */
  static std::unique_ptr<IntegerLiteral> FromStr(const std::string &Val);

 private:
  uint64_t Val_;
};

class StringLiteral : public Expr {
 public:
  StringLiteral(std::string &Val) : Val_(Val) {}

  // Return the raw string from the source code with the surrounding quotes.
  std::string Value() const { return Val_; }

  // Return the string without the surrounding quotes and escaped characters
  // (ie. "\n" is interpretted as the newline character in the resulting
  // string).
  std::string EscapedValue() const {
    std::string s;
    for (unsigned i = 1; i < Val_.size() - 1; ++i) {
      char c = Val_[i];
      if (c == '\\') {
        ++i;
        c = Val_[i];
        switch (c) {
          case 'n':
            s += '\n';
            break;
          default:
            s += c;
            break;
        }
      } else {
        s += c;
      }
    }
    return s;
  }

  ACCEPT_VISITORS;

 private:
  std::string Val_;
};

class ID : public Expr {
 public:
  ID(std::string &Name) : Name_(Name) {}

  std::string Name() const { return Name_; }

  ACCEPT_VISITORS;

 private:
  std::string Name_;
};

class Call : public Expr {
 public:
  Call(std::unique_ptr<Expr> Func) : Func_(std::move(Func)) {}
  Call(std::unique_ptr<Expr> Func, std::vector<std::unique_ptr<Expr>> &Args)
      : Func_(std::move(Func)), Args_(std::move(Args)) {}

  const Expr &Caller() const { return *Func_; }
  const std::vector<std::unique_ptr<Expr>> &Args() const { return Args_; }

  ACCEPT_VISITORS;

 private:
  std::unique_ptr<Expr> Func_;
  std::vector<std::unique_ptr<Expr>> Args_;
};

}  // namespace ast
}  // namespace lang

#endif
