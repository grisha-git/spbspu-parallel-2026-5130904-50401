#include "help_functions.hpp"

std::vector< std::string > donkeev::splitIntoTokens(const std::string& line)
{
  std::vector< std::string > tokens;
  size_t start = 0;
  size_t end = 0;

  while ((end = line.find(' ', start)) != std::string::npos)
  {
    if (end > start)
    {
      tokens.push_back(line.substr(start, end - start));
    }
    start = end + 1;
  }

  if (start < line.length())
  {
    tokens.push_back(line.substr(start));
  }

  return tokens;
}

bool donkeev::isNumber(const std::string& str)
{
  if (str.empty())
  {
    return false;
  }
  size_t start = 0;
  if (str[0] == '-' || str[0] == '+')
  {
    if (str.length() == 1)
    {
      return false;
    }
    start = 1;
  }
  for (size_t i = start; i < str.length(); ++i)
  {
    if (!std::isdigit(static_cast< unsigned char >(str[i])))
    {
      return false;
    }
  }
  return true;
}

bool donkeev::isPositiveNumber(const std::string& str)
{
  if (str.empty())
  {
    return false;
  }
  for (const char ch : str)
  {
    if (!std::isdigit(static_cast< unsigned char >(ch)))
    {
      return false;
    }
  }
  try
  {
    const unsigned long val = std::stoul(str);
    return val > 0;
  }
  catch (...)
  {
    return false;
  }
}

bool donkeev::isValidName(const std::string& str)
{
  if (str.empty())
  {
    return false;
  }
  const char first = str[0];
  if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z')))
  {
    return false;
  }
  return true;
}
