#ifndef ARGPARSER_H_
#define ARGPARSER_H_

#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
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
      std::vector<std::string>::const_iterator &args) const = 0;
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
      std::vector<std::string>::const_iterator &args) const override;
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
      std::vector<std::string>::const_iterator &args) const override;
};

// An argument that does not have a value passed to it.
// This is primarily used for arguments where you simply want to check if a
// flag is provided or not.
class EmptyArgument : public Argument {};

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

  template <class ArgTy>
  ArgTy GetArg(const std::string &argname) const {
    return *dynamic_cast<const ArgTy *>(args_.at(argname).get());
  }

  template <class ArgTy, class DefaultTy>
  ArgTy GetArg(const std::string &argname, DefaultTy default_arg) {
    if (HasArg(argname)) return GetArg<ArgTy>(argname);
    return ArgTy(default_arg);
  }

 private:
  std::unordered_map<std::string, std::unique_ptr<Argument>> args_;
};

struct KWArgParams {
  char short_argname;
};

class ArgParser {
 public:
  typedef std::unordered_map<std::string, std::unique_ptr<ArgParsingMethod>>
      ParsingMethods;
  typedef std::queue<std::pair<std::string, std::unique_ptr<ArgParsingMethod>>>
      PosParsingMethods;

  template <class ArgTy>
  void AddPositionalArgument(const std::string &argname) {
    pos_parsing_methods_.push(
        std::make_pair(argname, std::make_unique<ArgTy>()));
  }

  template <class ArgTy>
  void AddKeywordArgument(const std::string &argname) {
    struct KWArgParams default_params = {};
    AddKeywordArgument<ArgTy>(argname, default_params);
  }

  template <class ArgTy>
  void AddKeywordArgument(const std::string &argname,
                          const struct KWArgParams &kwparams) {
    parsing_methods_[argname] = std::make_unique<ArgTy>();
    HandleKWArgParams(argname, kwparams);
  }

  void AddEmptyKeywordArgument(const std::string &argname) {
    no_storage_args_.insert(argname);
    struct KWArgParams default_params = {};
    HandleKWArgParams(argname, default_params);
  }

  void AddEmptyKeywordArgument(const std::string &argname,
                               const struct KWArgParams &kwparams) {
    no_storage_args_.insert(argname);
    HandleKWArgParams(argname, kwparams);
  }

  ParsedArgs Parse(const std::vector<std::string> &args);
  ParsedArgs Parse(unsigned argc, char **argv);

  ArgParseStatus GetStatus() const { return parse_status_; }
  bool Ok() const { return parse_status_ == SUCCESS; }
  bool DebugOk() const;

 private:
  void HandleKWArgParams(const std::string &argname,
                         const struct KWArgParams &kwparams);

  ParsingMethods parsing_methods_;
  PosParsingMethods pos_parsing_methods_;
  std::unordered_map<char, std::string> short_argnames_;
  std::unordered_set<std::string> no_storage_args_;

  ArgParseStatus parse_status_ = SUCCESS;

  std::string unknown_arg_;
};

}  // namespace lang

#endif
