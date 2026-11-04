#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "shape_manager.hpp"
#include "process_manager.hpp"
#include "help_functions.hpp"
#include "commands.hpp"

int main()
{
  donkeev::ShapeManager shape_manager;
  donkeev::ProcessManager proc_manager;

  std::map< std::string, donkeev::Command > command_map;
  command_map.emplace("circle", donkeev::Command{donkeev::validateCircle, donkeev::executeCircle});
  command_map.emplace("set", donkeev::Command{donkeev::validateSet, donkeev::executeSet});
  command_map.emplace("show", donkeev::Command{donkeev::validateTwoTokens, donkeev::executeShow});
  command_map.emplace("showset", donkeev::Command{donkeev::validateTwoTokens, donkeev::executeShowSet});
  command_map.emplace("frame", donkeev::Command{donkeev::validateTwoTokens, donkeev::executeFrame});
  command_map.emplace("frameset", donkeev::Command{donkeev::validateTwoTokens, donkeev::executeFrameSet});
  command_map.emplace("spawn", donkeev::Command{donkeev::validateSpawn, donkeev::executeSpawn});
  command_map.emplace("area-on", donkeev::Command{donkeev::validateAreaOn, donkeev::executeAreaOn});
  command_map.emplace("status", donkeev::Command{donkeev::validateTwoTokens, donkeev::executeStatus});
  command_map.emplace("wait", donkeev::Command{donkeev::validateTwoTokens, donkeev::executeWait});

  std::string line;
  while (std::getline(std::cin, line))
  {
    if (line.empty())
    {
      continue;
    }

    auto tokens = donkeev::splitIntoTokens(line);
    if (tokens.empty())
    {
      continue;
    }

    const std::string& cmd_name = tokens[0];
    auto it = command_map.find(cmd_name);

    if (it == command_map.end())
    {
      std::cout << "<UNKNOWN COMMAND>\n";
      continue;
    }

    if (!it->second.validate(tokens))
    {
      std::cout << "<INVALID COMMAND>\n";
      continue;
    }

    try
    {
      it->second.execute(shape_manager, proc_manager, tokens);
    }
    catch (...)
    {
      std::cout << "<INVALID COMMAND>\n";
    }
  }

  return 0;
}
