#ifndef PARSER_HPP
#define PARSER_HPP
#include "commands/command.hpp"
#include "virtual_filesystem.hpp"
#include <map>
#include <memory>
#include <string>

class Parser {
public:
  Parser(VirtualFilesystem *vfs);
  void processCommand(const std::string &input);

private:
  std::map<std::string, std::unique_ptr<Command>> commands;
};

#endif