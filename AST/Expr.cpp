#include <string>

#include "Expr.h"

namespace lang {
namespace ast {

// TODO: This method does not allow fully parsing the largest possible
// decimal value for 2^64. Come back to this method to improve it.
static bool CanAlwaysFitInto64Bits(const std::string &Str) {
  return Str.length() <= 19;  // log10(2^64)
}

std::unique_ptr<IntegerLiteral> IntegerLiteral::FromStr(
    const std::string &Val) {
  if (!CanAlwaysFitInto64Bits(Val)) return nullptr;
  return std::make_unique<IntegerLiteral>(std::stoull(Val));
}

}  // namespace ast
}  // namespace lang
