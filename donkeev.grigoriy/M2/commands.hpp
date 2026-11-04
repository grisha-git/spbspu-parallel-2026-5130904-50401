#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <string>
#include <vector>
#include "shape_manager.hpp"
#include "process_manager.hpp"

namespace donkeev
{
  using ValidatorFunc = bool (*)(const std::vector< std::string >&);
  using ExecutorFunc = void (*)(ShapeManager&, ProcessManager&, const std::vector< std::string >&);

  struct Command
  {
    ValidatorFunc validate;
    ExecutorFunc execute;
  };

  bool validateCircle(const std::vector< std::string >& tokens);
  bool validateSet(const std::vector< std::string >& tokens);
  bool validateTwoTokens(const std::vector< std::string >&);
  bool validateSpawn(const std::vector< std::string >& tokens);
  bool validateAreaOn(const std::vector< std::string >& tokens);

  void executeCircle(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens);
  void executeSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens);
  void executeShow(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens);
  void executeShowSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens);
  void executeFrame(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens);
  void executeFrameSet(ShapeManager& manager, ProcessManager&, const std::vector< std::string >& tokens);

  void executeSpawn(ShapeManager&, ProcessManager& proc_manager, const std::vector< std::string >& tokens);
  void executeAreaOn(ShapeManager& manager, ProcessManager& proc_manager, const std::vector< std::string >& tokens);
  void executeStatus(ShapeManager&, ProcessManager& proc_manager, const std::vector< std::string >& tokens);
  void executeWait(ShapeManager&, ProcessManager& proc_manager, const std::vector< std::string >& tokens);
}

#endif
