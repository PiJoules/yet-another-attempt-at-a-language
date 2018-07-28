#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "ArgParser.h"

namespace lang {

namespace {

enum ParsedArgType {
  KEYWORD,
  SHORT_KEYWORD,
  POSITIONAL,
};

enum ParsedArgType GetArgType(const std::string &arg) {
  if (arg.size() >= 2 && arg[0] == '-') {
    if (arg[1] == '-')
      return KEYWORD;
    else if (std::isalpha(arg[1]))
      return SHORT_KEYWORD;
  }
  return POSITIONAL;
}

}  // namespace

ParsedArgs ArgParser::Parse(unsigned argc, char **argv) {
  std::vector<std::string> args(argv, argv + argc);
  return Parse(args);
}

static bool IsEmptyParsingMethod(const ArgParsingMethod *method) {
  return dynamic_cast<const EmptyParsingMethod *>(method) != nullptr;
}

ParsedArgs ArgParser::Parse(const std::vector<std::string> &args) {
  assert(args.size() >= 1 && "Expected at least one argument");

  ParsedArgs parsed_args;

  for (auto iter = args.cbegin() + 1; iter < args.cend();) {
    std::string argname = *iter;
    unknown_arg_ = argname;

    enum ParsedArgType argtype = GetArgType(argname);
    switch (argtype) {
      case POSITIONAL: {
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
      case SHORT_KEYWORD: {
        auto found_argname = short_argnames_.find(argname[1]);
        if (found_argname == short_argnames_.end()) {
          parse_status_ = NO_VALUE_FOR_FLAG;
          return parsed_args;
        }

        argname = found_argname->second;
        break;
      }
      case KEYWORD: {
        argname = argname.substr(2);
        break;
      }
    }
    unknown_arg_ = argname;

    if (parsing_methods_.find(argname) == parsing_methods_.end()) {
      parse_status_ = UNKNOWN_ARG;
      return parsed_args;
    }

    const ArgParsingMethod *parsing_method = parsing_methods_[argname].get();

    if (iter + 1 >= args.end() && !IsEmptyParsingMethod(parsing_method)) {
      parse_status_ = NO_VALUE_FOR_FLAG;
      return parsed_args;
    }

    std::unique_ptr<Argument> parsed_arg =
        parsing_method->ParseArgument(++iter);
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
    std::vector<std::string>::const_iterator &args) const {
  auto arg = std::make_unique<StringArgument>(*args);
  ++args;
  return arg;
}

std::unique_ptr<Argument> IntegerParsingMethod::ParseArgument(
    std::vector<std::string>::const_iterator &args) const {
  std::string strarg = *args;
  char *end = nullptr;
  long long val = std::strtoll(strarg.c_str(), &end, /*base=*/10);

  // Error since the last character is not the null terminator
  if (*end) return nullptr;

  auto arg = std::make_unique<IntegerArgument>(val);

  ++args;
  return arg;
}

std::unique_ptr<Argument> EmptyParsingMethod::ParseArgument(
    std::vector<std::string>::const_iterator &args) const {
  return std::make_unique<EmptyArgument>();
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
