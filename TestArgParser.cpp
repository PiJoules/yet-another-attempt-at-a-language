#include "ArgParser.h"
#include "gtest/gtest.h"

using namespace lang;

namespace {

TEST(TestArgParser, StringArgument) {
  std::vector<std::string> strargs = {
      "exe",
      "--foo",
      "arg",
  };
  ArgParser parser;
  parser.AddArgument<StringParsingMethod>("foo");

  ParsedArgs parsed_args = parser.Parse(strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_TRUE(parsed_args.HasArg("foo"));

  auto arg = parsed_args.GetArg<StringArgument>("foo");
  ASSERT_STREQ(arg.getValue().c_str(), "arg");
}

TEST(TestArgParser, IntegerArgument) {
  std::vector<std::string> strargs = {
      "exe",
      "--foo",
      "2",
  };
  ArgParser parser;
  parser.AddArgument<IntegerParsingMethod>("foo");

  ParsedArgs parsed_args = parser.Parse(strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_TRUE(parsed_args.HasArg("foo"));

  auto arg = parsed_args.GetArg<IntegerArgument>("foo");
  ASSERT_EQ(arg.getValue(), 2);
}

TEST(TestArgParser, UnusedArgs) {
  std::vector<std::string> strargs = {
      "exe",
  };
  ArgParser parser;
  parser.AddArgument<IntegerParsingMethod>("foo");

  ParsedArgs parsed_args = parser.Parse(strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_FALSE(parsed_args.HasArg("foo"));
}

TEST(TestArgParser, PositionalArgument) {
  std::vector<std::string> strargs = {
      "exe",
      "arg",
      "2",
  };
  ArgParser parser;
  parser.AddArgument<StringParsingMethod>("foo", /*positional=*/true);
  parser.AddArgument<IntegerParsingMethod>("bar", /*positional=*/true);

  ParsedArgs parsed_args = parser.Parse(strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_TRUE(parsed_args.HasArg("foo"));
  ASSERT_TRUE(parsed_args.HasArg("bar"));

  auto arg = parsed_args.GetArg<StringArgument>("foo");
  ASSERT_STREQ(arg.getValue().c_str(), "arg");

  auto arg2 = parsed_args.GetArg<IntegerArgument>("bar");
  ASSERT_EQ(arg2.getValue(), 2);
}

TEST(TestArgParser, ArgcArgvArgs) {
  char arg1[] = "exe";
  char arg2[] = "arg";
  char arg3[] = "--foo";
  char arg4[] = "2";
  char *strargs[] = {arg1, arg2, arg3, arg4, nullptr};

  ArgParser parser;
  parser.AddArgument<IntegerParsingMethod>("foo");
  parser.AddArgument<StringParsingMethod>("bar", /*positional=*/true);

  ParsedArgs parsed_args = parser.Parse(/*argc=*/4, strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_TRUE(parsed_args.HasArg("foo"));
  ASSERT_TRUE(parsed_args.HasArg("bar"));

  auto parsed_arg = parsed_args.GetArg<StringArgument>("bar");
  ASSERT_STREQ(parsed_arg.getValue().c_str(), "arg");

  auto parsed_arg2 = parsed_args.GetArg<IntegerArgument>("foo");
  ASSERT_EQ(parsed_arg2.getValue(), 2);
}

TEST(TestArgParser, DefaultArg) {
  std::vector<std::string> strargs = {
      "exe",
  };
  ArgParser parser;
  parser.AddArgument<StringParsingMethod>("foo");
  parser.AddArgument<IntegerParsingMethod>("bar", /*positional=*/true);

  ParsedArgs parsed_args = parser.Parse(strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_FALSE(parsed_args.HasArg("foo"));
  ASSERT_FALSE(parsed_args.HasArg("bar"));

  auto foo = parsed_args.GetArg<StringArgument>("foo", "abc");
  ASSERT_STREQ(foo.getValue().c_str(), "abc");

  auto bar = parsed_args.GetArg<IntegerArgument>("foo", 2);
  ASSERT_EQ(bar.getValue(), 2);
}

TEST(TestArgParser, ShortArgName) {
  std::vector<std::string> strargs = {
      "exe",
      "-f",
      "abc",
  };
  ArgParser parser;
  parser.AddKeywordArgument<StringParsingMethod>("foo", 'f');

  ParsedArgs parsed_args = parser.Parse(strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_TRUE(parsed_args.HasArg("foo"));

  auto foo = parsed_args.GetArg<StringArgument>("foo");
  ASSERT_STREQ(foo.getValue().c_str(), "abc");
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
