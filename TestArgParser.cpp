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
  parser.AddArgument<StringParsingMethod>("--foo");

  ParsedArgs parsed_args = parser.Parse(strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_TRUE(parsed_args.HasArg("--foo"));

  const auto &arg =
      dynamic_cast<const StringArgument &>(parsed_args.GetArg("--foo"));
  ASSERT_STREQ(arg.getValue().c_str(), "arg");
}

TEST(TestArgParser, IntegerArgument) {
  std::vector<std::string> strargs = {
      "exe",
      "--foo",
      "2",
  };
  ArgParser parser;
  parser.AddArgument<IntegerParsingMethod>("--foo");

  ParsedArgs parsed_args = parser.Parse(strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_TRUE(parsed_args.HasArg("--foo"));

  const auto &arg =
      dynamic_cast<const IntegerArgument &>(parsed_args.GetArg("--foo"));
  ASSERT_EQ(arg.getValue(), 2);
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

  const auto &arg =
      dynamic_cast<const StringArgument &>(parsed_args.GetArg("foo"));
  ASSERT_STREQ(arg.getValue().c_str(), "arg");

  const auto &arg2 =
      dynamic_cast<const IntegerArgument &>(parsed_args.GetArg("bar"));
  ASSERT_EQ(arg2.getValue(), 2);
}

TEST(TestArgParser, ArgcArgvArgs) {
  char arg1[] = "exe";
  char arg2[] = "arg";
  char arg3[] = "--foo";
  char arg4[] = "2";
  char *strargs[] = {arg1, arg2, arg3, arg4, nullptr};

  ArgParser parser;
  parser.AddArgument<IntegerParsingMethod>("--foo");
  parser.AddArgument<StringParsingMethod>("bar", /*positional=*/true);

  ParsedArgs parsed_args = parser.Parse(/*argc=*/4, strargs);
  ASSERT_TRUE(parser.DebugOk());
  ASSERT_TRUE(parsed_args.HasArg("--foo"));
  ASSERT_TRUE(parsed_args.HasArg("bar"));

  const auto &parsed_arg =
      dynamic_cast<const StringArgument &>(parsed_args.GetArg("bar"));
  ASSERT_STREQ(parsed_arg.getValue().c_str(), "arg");

  const auto &parsed_arg2 =
      dynamic_cast<const IntegerArgument &>(parsed_args.GetArg("--foo"));
  ASSERT_EQ(parsed_arg2.getValue(), 2);
}

}  // namespace

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
