#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "ArgParser.h"

namespace lang {

static bool IsKeywordArg(const std::string &arg) {
  return arg.size() >= 2 && arg[0] == '-' && arg[1] == '-';
}

static bool IsPosArg(const std::string &arg) { return !IsKeywordArg(arg); }

void ArgParser::AddStringArgument(const std::string &argname, bool positional) {
  if (positional)
    pos_parsing_methods_.push(
        std::make_pair(argname, std::make_unique<StringParsingMethod>()));
  else
    parsing_methods_[argname] = std::make_unique<StringParsingMethod>();
}

void ArgParser::AddIntegerArgument(const std::string &argname,
                                   bool positional) {
  if (positional)
    pos_parsing_methods_.push(
        std::make_pair(argname, std::make_unique<IntegerParsingMethod>()));
  else
    parsing_methods_[argname] = std::make_unique<IntegerParsingMethod>();
}

ParsedArgs ArgParser::Parse(unsigned argc, char **argv) {
  std::vector<std::string> args(argv, argv + argc);
  return Parse(args);
}

ParsedArgs ArgParser::Parse(const std::vector<std::string> &args) {
  assert(args.size() >= 1 && "Expected at least one argument");

  ParsedArgs parsed_args;

  for (auto iter = args.cbegin() + 1; iter < args.cend();) {
    std::string argname = *iter;
    unknown_arg_ = argname;

    if (IsPosArg(argname)) {
      if (pos_parsing_methods_.empty()) {
        parse_status_ = NO_VALUE_FOR_POS_ARG;
        return parsed_args;
      }

      auto parsed_arg_pair = std::move(pos_parsing_methods_.front());
      pos_parsing_methods_.pop();

      std::unique_ptr<Argument> parsed_arg =
          parsed_arg_pair.second->ParseArgument(iter);
      if (!parsed_arg) {
        parse_status_ = NO_VALUE_FOR_POS_ARG;
        return parsed_args;
      }
      parsed_args.SetArg(parsed_arg_pair.first, std::move(parsed_arg));
      continue;
    }

    if (parsing_methods_.find(argname) == parsing_methods_.end()) {
      parse_status_ = UNKNOWN_ARG;
      return parsed_args;
    }

    if (iter + 1 >= args.end()) {
      parse_status_ = NO_VALUE_FOR_FLAG;
      return parsed_args;
    }

    std::unique_ptr<Argument> parsed_arg =
        parsing_methods_[argname]->ParseArgument(++iter);
    if (!parsed_arg) {
      parse_status_ = PARSE_ERROR;
      return parsed_args;
    }

    parsed_args.SetArg(argname, std::move(parsed_arg));
    ++iter;
  }

  parse_status_ = SUCCESS;
  return parsed_args;
}

std::unique_ptr<Argument> StringParsingMethod::ParseArgument(
    std::vector<std::string>::const_iterator &args) {
  auto arg = std::make_unique<StringArgument>(*args);
  ++args;
  return arg;
}

std::unique_ptr<Argument> IntegerParsingMethod::ParseArgument(
    std::vector<std::string>::const_iterator &args) {
  std::string strarg = *args;
  char *end = nullptr;
  long long val = std::strtoll(strarg.c_str(), &end, /*base=*/10);

  // Error since the last character is not the null terminator
  if (*end) return nullptr;

  auto arg = std::make_unique<IntegerArgument>(val);

  ++args;
  return arg;
}

bool ArgParser::DebugOk() const {
  if (parse_status_ == SUCCESS) return true;

  switch (parse_status_) {
    case SUCCESS:
      return true;
    case UNKNOWN_ARG:
      std::cerr << "Parsed unknown argument \"" << unknown_arg_ << "\""
                << std::endl;
      return false;
    case NO_VALUE_FOR_FLAG:
      std::cerr << "No value found for flag \"" << unknown_arg_ << "\""
                << std::endl;
      return false;
    case PARSE_ERROR:
      std::cerr << "Error parsing for flag \"" << unknown_arg_ << "\""
                << std::endl;
      return false;
    case NO_VALUE_FOR_POS_ARG:
      std::cerr << "No storage for positional argument \"" << unknown_arg_
                << "\"" << std::endl;
      return false;
  }
}

}  // namespace lang
