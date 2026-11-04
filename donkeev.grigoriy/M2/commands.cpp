#include "commands.hpp"
#include "help_functions.hpp"
#include <iostream>
#include <stdexcept>

namespace
{
  constexpr size_t kCircleTokens = 5;
  constexpr size_t kAreaOnTokens = 6;
  constexpr size_t kTriesIndex = 5;
}

bool donkeev::validateCircle(const std::vector< std::string >& tokens)
{
  return tokens.size() == kCircleTokens && isValidName(tokens[1]) && isPositiveNumber(tokens[2]) && isNumber(tokens[3])
      && isNumber(tokens[4]);
}

bool donkeev::validateSet(const std::vector< std::string >& tokens)
{
  if (tokens.size() < 3 || !isValidName(tokens[1]) || !isPositiveNumber(tokens[2]))
  {
    return false;
  }
  try
  {
    const size_t set_size = std::stoul(tokens[2]);
    if (tokens.size() != 3 + set_size)
    {
      return false;
    }
    for (size_t i = 3; i < tokens.size(); ++i)
    {
      if (!isValidName(tokens[i]))
      {
        return false;
      }
    }
    return true;
  }
  catch (...)
  {
    return false;
  }
}

bool donkeev::validateTwoTokens(const std::vector< std::string >& tokens)
{
  return tokens.size() == 2 && isValidName(tokens[1]);
}

bool donkeev::validateSpawn(const std::vector< std::string >& tokens)
{
  return tokens.size() == 3 && isValidName(tokens[1]) && isNumber(tokens[2]);
}

bool donkeev::validateAreaOn(const std::vector< std::string >& tokens)
{
  return tokens.size() == kAreaOnTokens && isValidName(tokens[1]) && isValidName(tokens[2]) && isValidName(tokens[3])
      && isPositiveNumber(tokens[4]) && isPositiveNumber(tokens[kTriesIndex]);
}

void donkeev::executeCircle(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  manager.addCircle(tokens[1], std::stoi(tokens[2]), std::stoi(tokens[3]), std::stoi(tokens[4]));
}

void donkeev::executeSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  const size_t size = std::stoul(tokens[2]);
  const std::vector< std::string > shape_names(tokens.begin() + 3, tokens.begin() + 3 + size);
  manager.addSet(tokens[1], shape_names);
}

void donkeev::executeShow(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  const auto& circle = manager.getCircle(tokens[1]);
  auto center = circle.getCenter();
  std::cout << circle.getRadius() << " (" << center.x << " " << center.y << ")\n";
}

void donkeev::executeShowSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  const auto& shape_set = manager.getSet(tokens[1]);
  for (const auto& shape_name : shape_set.shape_names)
  {
    const auto& circle = manager.getCircle(shape_name);
    auto center = circle.getCenter();
    std::cout << circle.getRadius() << " (" << center.x << " " << center.y << ")\n";
  }
}

void donkeev::executeFrame(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  const auto& circle = manager.getCircle(tokens[1]);
  const rectangle_t bbox = circle.getBoundingBox();
  std::cout << "(" << bbox.bottom_left.x << " " << bbox.bottom_left.y << ") "
            << "(" << bbox.top_right.x << " " << bbox.top_right.y << ")\n";
}

void donkeev::executeFrameSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens)
{
  const rectangle_t bbox = manager.getSetBoundingBox(tokens[1]);
  std::cout << "(" << bbox.bottom_left.x << " " << bbox.bottom_left.y << ") "
            << "(" << bbox.top_right.x << " " << bbox.top_right.y << ")\n";
}

void donkeev::executeSpawn(ShapeManager&, ProcessManager& proc_manager, const std::vector< std::string >& tokens)
{
  const std::string& name = tokens[1];
  const long long seed_input = std::stoll(tokens[2]);
  if (seed_input < 0)
  {
    throw std::invalid_argument("Seed cannot be negative");
  }

  if (proc_manager.hasProcess(name))
  {
    throw std::invalid_argument("Process already exists");
  }

  const size_t seed = static_cast< size_t >(seed_input);
  const std::vector< Process > existing_processes = proc_manager.getAllProcesses();

  Process process = spawnProcess(seed, existing_processes);
  try
  {
    proc_manager.addProcess(name, process);
  }
  catch (...)
  {
    closeProcess(process);
    throw;
  }
}

void donkeev::executeAreaOn(
    ShapeManager& manager, ProcessManager& proc_manager, const std::vector< std::string >& tokens)
{
  const std::string& process_name = tokens[1];
  const std::string& calc_name = tokens[2];
  const std::string& set_name = tokens[3];
  const size_t threads = std::stoul(tokens[4]);
  const size_t tries = std::stoul(tokens[kTriesIndex]);

  proc_manager.startCalculation(process_name, calc_name, set_name, manager, threads, tries);
}

void donkeev::executeStatus(ShapeManager&, ProcessManager& proc_manager, const std::vector< std::string >& tokens)
{
  proc_manager.printStatus(tokens[1], std::cout);
}

void donkeev::executeWait(ShapeManager&, ProcessManager& proc_manager, const std::vector< std::string >& tokens)
{
  proc_manager.waitCalculation(tokens[1], std::cout);
}
