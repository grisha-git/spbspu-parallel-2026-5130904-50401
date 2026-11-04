#ifndef HELP_FUNCTIONS_HPP
#define HELP_FUNCTIONS_HPP

#include <vector>
#include <string>

namespace donkeev
{
  std::vector< std::string > splitIntoTokens(const std::string &);
  bool isNumber(const std::string &);
  bool isPositiveNumber(const std::string &);
  bool isValidName(const std::string &);
}

#endif
