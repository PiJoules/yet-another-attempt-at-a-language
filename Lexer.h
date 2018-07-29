#ifndef LEXER_H_
#define LEXER_H_

#include <cstdio>
#include <iostream>
#include <istream>
#include <string>

namespace lang {
enum TokenKind {
  TOK_UNKNOWN = 0,
  TOK_EOF,

  // Keywords
  // NOTE: New keywords added here MUST be added to the function
  // `TokenKindFromStr()` that handles conversion from a TokenKind to a string
  TOK_RETURN,

  // Atoms
  TOK_ID,  // Could refer to type or variable
  TOK_INT,
  TOK_STR,

  // New non-alpha numeric characters added here will need to be checked in
  // Lexer::ReadToken()
  TOK_SEMICOL,
  TOK_COL,
  TOK_COMMA,
  TOK_LPAR,
  TOK_RPAR,
  TOK_LBRACE,  // {
  TOK_RBRACE,  // }
  TOK_DQUOTE,
  TOK_ASSIGN,
};

struct Token {
  enum TokenKind Kind;
  std::string Chars;
  unsigned Row;
  unsigned Col;

  void dump(std::ostream &out) const {
    std::string chars = (Kind == TOK_EOF) ? "EOF" : Chars;
    out << "<" << Kind << " " << Row << ":" << Col << " '" << chars << "'>";
  }
};

class Lexer {
 public:
  /**
   * The file must have, and assumes, an open mode of `in` which cannot be
   * checked for. Undefined behavior occurs if an iopstream is passed not
   * operating on `in` mode.
   */
  explicit Lexer(std::istream &Input) : Input_(Input) {}

  /**
   * Read a token off the stream. Returns true if a token was successfully read.
   * Returns false if we ran into an unknown/unhandled character in the stream.
   *
   * This stream considers reading EOF a successful operation. In this case, if
   * the intended use is to stop after EOF, just check the token kind and stop
   * calling this method.
   *
   * The stream is advanced and the character read from the stream. The token
   * members can still be overwritten on a failed read.
   */
  bool ReadToken(Token &Tok);

  /**
   * Same as ReadToken() but does not advance the stream of tokens. The internal
   * stream may be advanced and the row and col are updated.
   */
  bool PeekToken(Token &Tok);

  bool ReachedEOF() {
    int C;
    while (SafePeek(C) && isspace(C)) ReadCharAndUpdatePos();
    return Input_.eof() || (SafePeek(C) && C == EOF);
  }

  /**
   * This is the character that we were unable to handle and is still left on
   * the stream.
   */
  int CharReadOnErr() const { return CharReadOnErr_; }

  /**
   * These always point to the current row and column after every
   * Read/PeekToken() call. This can also be used on unsuccessful reads to get
   * the location of an unhandled character. These values start at zero to
   * represent the first row/col.
   */
  unsigned Row() const { return Row_; }
  unsigned Col() const { return Col_; }

 private:
  /**
   * "Safe" wrappers for stream functions that check if the stream failed after
   * each operation. Returns true on successful operation and false on a failed
   * one.
   */
  bool SafePeek(int &C) {
    if (Input_.eof()) {
      C = EOF;
      return true;
    }
    C = Input_.peek();
    return !Input_.fail();
  }

  int ReadCharAndUpdatePos() {
    int C = Input_.get();
    if (C == '\n') {
      ++Row_;
      Col_ = 0;
    } else if (C != EOF) {
      ++Col_;
    }
    return C;
  }

  void ExhaustWhitespace() {
    int C;
    while (SafePeek(C) && isspace(C)) ReadCharAndUpdatePos();
  }

  inline void SaveErrData(int C) { CharReadOnErr_ = C; }

  std::istream &Input_;
  unsigned Row_ = 0;
  unsigned Col_ = 0;

  int CharReadOnErr_;
  Token BufferedTok_;
  bool HasBufferedTok_ = false;
};
}  // namespace lang

#endif
