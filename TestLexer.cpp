#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>

#include "Lexer.h"
#include "gtest/gtest.h"

#define TEST_SINGLE_TOKEN(STR, KIND, TEST_NAME) \
  TEST_F(LexerTest, TEST_NAME) {                \
    Input_ << STR;                              \
    Lexer Lex(Input_);                          \
    Token Tok;                                  \
    ASSERT_TRUE(Lex.ReadToken(Tok));            \
    ASSERT_STREQ(Tok.Chars.c_str(), STR);       \
    ASSERT_EQ(Tok.Kind, KIND);                  \
    ASSERT_EQ(Tok.Row, 0);                      \
    ASSERT_EQ(Tok.Col, 0);                      \
    ASSERT_EQ(Lex.Row(), 0);                    \
    ASSERT_EQ(Lex.Col(), Tok.Chars.length());   \
    unsigned Len = Tok.Chars.length();          \
    ASSERT_TRUE(Lex.ReadToken(Tok));            \
    ASSERT_EQ(Tok.Kind, lang::TOK_EOF);         \
    ASSERT_EQ(Tok.Row, 0);                      \
    ASSERT_EQ(Tok.Col, Len);                    \
    ASSERT_EQ(Lex.Row(), 0);                    \
    ASSERT_EQ(Lex.Col(), Len);                  \
  }

using lang::Lexer;
using lang::Token;

namespace {

class LexerTest : public ::testing::Test {
 protected:
  std::stringstream Input_;
};

TEST_F(LexerTest, ValidFile) {
  Input_ << "a";
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);
  ASSERT_EQ(Lex.Row(), 0);
  ASSERT_EQ(Lex.Col(), 1);
}

TEST_F(LexerTest, EmptyFile) {
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);
  ASSERT_EQ(Lex.Row(), 0);
  ASSERT_EQ(Lex.Col(), 0);
}

TEST_F(LexerTest, MultipleEOFReads) {
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);
  ASSERT_EQ(Lex.Row(), 0);
  ASSERT_EQ(Lex.Col(), 0);
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);
  ASSERT_EQ(Lex.Row(), 0);
  ASSERT_EQ(Lex.Col(), 0);
}

TEST_F(LexerTest, NewlineRowUpdate) {
  Input_ << "a\n"
         << "bc d\n"
         << "\n";
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 0);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 3);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
  ASSERT_EQ(Lex.Row(), 3);
  ASSERT_EQ(Lex.Col(), 0);
}

TEST_SINGLE_TOKEN("abcde123", lang::TOK_ID, ReadID)
TEST_SINGLE_TOKEN("Return", lang::TOK_ID, CaseSensitiveKeyword)
TEST_SINGLE_TOKEN("return123", lang::TOK_ID, WholeWordGrabbed)

TEST_SINGLE_TOKEN("return", lang::TOK_RETURN, ReadReturn)

TEST_SINGLE_TOKEN("128", lang::TOK_INT, ReadInt)

TEST_SINGLE_TOKEN(";", lang::TOK_SEMICOL, ReadSemicol)
TEST_SINGLE_TOKEN(",", lang::TOK_COMMA, ReadComma)
TEST_SINGLE_TOKEN("(", lang::TOK_LPAR, ReadLPar)
TEST_SINGLE_TOKEN(")", lang::TOK_RPAR, ReadRPar)
TEST_SINGLE_TOKEN("{", lang::TOK_LBRACE, ReadLBrace)
TEST_SINGLE_TOKEN("}", lang::TOK_RBRACE, ReadRBrace)

TEST_SINGLE_TOKEN("\"abcde\"", lang::TOK_STR, ReadStr)
TEST_SINGLE_TOKEN("\"ab cd e\"", lang::TOK_STR, ReadStrWithSpaces)
TEST_SINGLE_TOKEN("\"ab \\n e\"", lang::TOK_STR, ReadStrWithEscape)
TEST_SINGLE_TOKEN("\"a \\\"b \\\" c\"", lang::TOK_STR, ReadStrWithEscapedQuote)

TEST_F(LexerTest, SkipWhitespace) {
  Input_ << "return var";
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RETURN);
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_STREQ(Tok.Chars.c_str(), "var");
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
}

TEST_F(LexerTest, SkipWhitespaceAtStart) {
  Input_ << "   return";
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RETURN);
}

TEST_F(LexerTest, SkipWhitespaceAtEnd) {
  Input_ << "return   ";
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RETURN);
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
}

TEST_F(LexerTest, OnlyWhitespace) {
  Input_ << "        ";
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
}

TEST_F(LexerTest, UnknownLookahead) {
  Input_ << static_cast<char>(128);
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_FALSE(Lex.ReadToken(Tok));
  ASSERT_EQ(Lex.CharReadOnErr(), 128);
}

TEST_F(LexerTest, LexErrorLoc) {
  Input_ << "\nab " << static_cast<char>(128) << "cd";
  Lexer Lex(Input_);
  Token Tok;

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);

  ASSERT_FALSE(Lex.ReadToken(Tok));
  ASSERT_EQ(Lex.CharReadOnErr(), 128);
  ASSERT_EQ(Lex.Row(), 1);
  ASSERT_EQ(Lex.Col(), 3);

  ASSERT_FALSE(Lex.ReadToken(Tok));
  ASSERT_EQ(Lex.CharReadOnErr(), 128);
  ASSERT_EQ(Lex.Row(), 1);
  ASSERT_EQ(Lex.Col(), 3);
}

TEST_F(LexerTest, HelloWorld) {
  Input_ << "int main() {\n"
            "  printf(\"hello world\\n\");\n"
            "  return 0;\n"
            "}";
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_STREQ(Tok.Chars.c_str(), "int");
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_STREQ(Tok.Chars.c_str(), "main");
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 4);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_LPAR);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 8);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RPAR);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 9);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_LBRACE);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 11);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_STREQ(Tok.Chars.c_str(), "printf");
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 2);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_LPAR);
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 8);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_STR);
  ASSERT_STREQ(Tok.Chars.c_str(), "\"hello world\\n\"");
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 9);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RPAR);
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 24);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_SEMICOL);
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 25);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RETURN);
  ASSERT_EQ(Tok.Row, 2);
  ASSERT_EQ(Tok.Col, 2);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_INT);
  ASSERT_STREQ(Tok.Chars.c_str(), "0");
  ASSERT_EQ(Tok.Row, 2);
  ASSERT_EQ(Tok.Col, 9);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_SEMICOL);
  ASSERT_EQ(Tok.Row, 2);
  ASSERT_EQ(Tok.Col, 10);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RBRACE);
  ASSERT_EQ(Tok.Row, 3);
  ASSERT_EQ(Tok.Col, 0);

  ASSERT_TRUE(Lex.ReachedEOF());
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
  ASSERT_TRUE(Lex.ReachedEOF());
}

TEST_F(LexerTest, ReachedEOF) {
  Lexer Lex(Input_);
  ASSERT_TRUE(Lex.ReachedEOF());
}

TEST_F(LexerTest, PeekToken) {
  Input_ << "return var";
  Lexer Lex(Input_);
  Token Tok;
  ASSERT_TRUE(Lex.PeekToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RETURN);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RETURN);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);

  ASSERT_TRUE(Lex.PeekToken(Tok));
  ASSERT_STREQ(Tok.Chars.c_str(), "var");
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 7);
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_STREQ(Tok.Chars.c_str(), "var");
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 7);

  ASSERT_TRUE(Lex.PeekToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
}

TEST_F(LexerTest, HelloWorldFromFile) {
  std::ifstream Input("examples/hello_world.lang");
  Lexer Lex(Input);
  Token Tok;
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_STREQ(Tok.Chars.c_str(), "int");
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 0);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_STREQ(Tok.Chars.c_str(), "main");
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 4);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_LPAR);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 8);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RPAR);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 9);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_LBRACE);
  ASSERT_EQ(Tok.Row, 0);
  ASSERT_EQ(Tok.Col, 11);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_ID);
  ASSERT_STREQ(Tok.Chars.c_str(), "printf");
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 2);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_LPAR);
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 8);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_STR);
  ASSERT_STREQ(Tok.Chars.c_str(), "\"hello world\\n\"");
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 9);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RPAR);
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 24);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_SEMICOL);
  ASSERT_EQ(Tok.Row, 1);
  ASSERT_EQ(Tok.Col, 25);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RETURN);
  ASSERT_EQ(Tok.Row, 2);
  ASSERT_EQ(Tok.Col, 2);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_INT);
  ASSERT_STREQ(Tok.Chars.c_str(), "0");
  ASSERT_EQ(Tok.Row, 2);
  ASSERT_EQ(Tok.Col, 9);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_SEMICOL);
  ASSERT_EQ(Tok.Row, 2);
  ASSERT_EQ(Tok.Col, 10);

  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_RBRACE);
  ASSERT_EQ(Tok.Row, 3);
  ASSERT_EQ(Tok.Col, 0);

  ASSERT_TRUE(Lex.ReachedEOF());
  ASSERT_TRUE(Lex.ReadToken(Tok));
  ASSERT_EQ(Tok.Kind, lang::TOK_EOF);
  ASSERT_TRUE(Lex.ReachedEOF());
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
