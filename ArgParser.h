#ifndef ARGPARSER_H_
#define ARGPARSER_H_

#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

namespace lang {

class Argument {
 public:
  virtual ~Argument() {}
};
class ArgParsingMethod {
 public:
  virtual ~ArgParsingMethod() {}

  virtual std::unique_ptr<Argument> ParseArgument(
      std::vector<std::string>::const_iterator &args) = 0;
};

class StringArgument : public Argument {
 public:
  StringArgument(const std::string &val) : val_(val) {}

  std::string getValue() const { return val_; }

 private:
  std::string val_;
};
class StringParsingMethod : public ArgParsingMethod {
 public:
  virtual std::unique_ptr<Argument> ParseArgument(
      std::vector<std::string>::const_iterator &args) override;
};

class IntegerArgument : public Argument {
 public:
  IntegerArgument(int64_t val) : val_(val) {}

  int64_t getValue() const { return val_; }

 private:
  int64_t val_;
};
class IntegerParsingMethod : public ArgParsingMethod {
 public:
  virtual std::unique_ptr<Argument> ParseArgument(
      std::vector<std::string>::const_iterator &args) override;
};

enum ArgParseStatus {
  SUCCESS,
  UNKNOWN_ARG,
  NO_VALUE_FOR_FLAG,
  NO_VALUE_FOR_POS_ARG,
  PARSE_ERROR,
};

class ParsedArgs {
 public:
  void SetArg(const std::string &argname, std::unique_ptr<Argument> arg) {
    args_[argname] = std::move(arg);
  }

  bool HasArg(const std::string &argname) const {
    return args_.find(argname) != args_.end();
  }

  const Argument &GetArg(const std::string &argname) const {
    return *(args_.at(argname).get());
  }

  template <class ArgTy>
  const ArgTy &GetArg(const std::string &argname) const {
    return dynamic_cast<const ArgTy &>(GetArg(argname));
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<Argument>> args_;
};

class ArgParser {
 public:
  typedef std::unordered_map<std::string, std::unique_ptr<ArgParsingMethod>>
      ParsingMethods;
  typedef std::queue<std::pair<std::string, std::unique_ptr<ArgParsingMethod>>>
      PosParsingMethods;

  template <class ArgTy>
  void AddArgument(const std::string &argname, bool positional = false) {
    if (positional)
      pos_parsing_methods_.push(
          std::make_pair(argname, std::make_unique<ArgTy>()));
    else
      parsing_methods_[argname] = std::make_unique<ArgTy>();
  }

  ParsedArgs Parse(const std::vector<std::string> &args);
  ParsedArgs Parse(unsigned argc, char **argv);

  ArgParseStatus GetStatus() const { return parse_status_; }
  bool Ok() const { return parse_status_ == SUCCESS; }
  bool DebugOk() const;

 private:
  ParsingMethods parsing_methods_;
  PosParsingMethods pos_parsing_methods_;

  ArgParseStatus parse_status_ = SUCCESS;

  std::string unknown_arg_;
};

}  // namespace lang

#endif
