#pragma once
#include "commands/command.hpp"
#include "virtual_filesystem.hpp"
#include <map>
#include <memory>
#include <string>

class Parser {
public:
  explicit Parser(std::shared_ptr<VirtualFilesystem> vfs);
  std::string processCommand(const std::string &input);
private:
  std::unordered_map<std::string, std::unique_ptr<Command>> commands;
};
