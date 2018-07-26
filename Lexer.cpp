#include <cassert>

#include "Lexer.h"

namespace lang {
/**
 * NOTE: This function assumes the argument passed is an alphanumeric string.
 */
static enum TokenKind TokenKindFromStr(const std::string &Keyword) {
  if (Keyword == "return") return TOK_RETURN;
  return TOK_UNKNOWN;
}

static void SetTokenEOF(Token &Tok) {
  Tok.Chars.clear();
  Tok.Chars += EOF;
  Tok.Kind = TOK_EOF;
}

bool Lexer::ReadToken(Token &Tok) {
  if (HasBufferedTok_) {
    HasBufferedTok_ = false;
    Tok = BufferedTok_;
    return true;
  }

  // Early EOF detection
  Tok.Row = Row();
  Tok.Col = Col();
  if (Input_.eof() && !Input_.fail()) {
    SetTokenEOF(Tok);
    return true;
  }

  int C;
  while (SafePeek(C) && isspace(C)) ReadCharAndUpdatePos();
  if (Input_.fail()) {
    std::cerr << "Failed after clearing whitespace" << std::endl;
    SaveErrData(C);
    return false;
  }

  // Overwrite whitespace location
  Tok.Row = Row();
  Tok.Col = Col();

  if (isalpha(C)) {
    // Reading an ID, type, or keyword
    std::string Chars;
    while (SafePeek(C) && isalnum(C)) Chars += ReadCharAndUpdatePos();
    if (Input_.fail()) {
      SaveErrData(C);
      return false;
    }

    enum TokenKind Kind = TokenKindFromStr(Chars);
    if (Kind == TOK_UNKNOWN) {
      Tok.Kind = TOK_ID;
    } else {
      Tok.Kind = Kind;
    }
    Tok.Chars = Chars;

    return true;
  } else if (isdigit(C)) {
    // Int literal
    std::string Chars;
    while (SafePeek(C) && isdigit(C)) Chars += ReadCharAndUpdatePos();
    if (Input_.fail()) {
      SaveErrData(C);
      return false;
    }

    Tok.Chars = Chars;
    Tok.Kind = TOK_INT;
    return true;
  } else if (C == '"') {
    // Read off a string
    std::string Chars;
    Chars += ReadCharAndUpdatePos();
    bool OnEscapedQuote = false;
    while (SafePeek(C) && (C != '"' || OnEscapedQuote)) {
      Chars += ReadCharAndUpdatePos();
      int Lookahead;
      SafePeek(Lookahead);
      OnEscapedQuote = (C == '\\' && Lookahead == '"');
    }
    if (Input_.fail()) {
      SaveErrData(C);
      return false;
    }
    Chars += ReadCharAndUpdatePos();
    Tok.Chars = Chars;
    Tok.Kind = TOK_STR;
    return true;
  }

  switch (C) {
    case ';':
      ReadCharAndUpdatePos();
      Tok.Chars = ";";
      Tok.Kind = TOK_SEMICOL;
      return true;
    case ',':
      ReadCharAndUpdatePos();
      Tok.Chars = ",";
      Tok.Kind = TOK_COMMA;
      return true;
    case '(':
      ReadCharAndUpdatePos();
      Tok.Chars = "(";
      Tok.Kind = TOK_LPAR;
      return true;
    case ')':
      ReadCharAndUpdatePos();
      Tok.Chars = ")";
      Tok.Kind = TOK_RPAR;
      return true;
    case '"':
      ReadCharAndUpdatePos();
      Tok.Chars = "\"";
      Tok.Kind = TOK_DQUOTE;
      return true;
    case '{':
      ReadCharAndUpdatePos();
      Tok.Chars = "{";
      Tok.Kind = TOK_LBRACE;
      return true;
    case '}':
      ReadCharAndUpdatePos();
      Tok.Chars = "}";
      Tok.Kind = TOK_RBRACE;
      return true;
    case EOF:
      SetTokenEOF(Tok);
      return true;
    default:
      // Unknown lookahead
      SaveErrData(C);
      return false;
  }
}

bool Lexer::PeekToken(Token &Tok) {
  if (!HasBufferedTok_) {
    HasBufferedTok_ = ReadToken(BufferedTok_);
  }
  Tok = BufferedTok_;
  return HasBufferedTok_ && !Input_.fail();
}
}  // namespace lang
