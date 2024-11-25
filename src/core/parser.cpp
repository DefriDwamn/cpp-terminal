#include "core/parser.hpp"
#include <iostream>
#include <memory>
#include <sstream>

Parser::Parser(std::shared_ptr<VirtualFilesystem> vfs) {
  commands["cd"] = std::make_unique<ChangeDirectoryCommand>(vfs);
  commands["ls"] = std::make_unique<ListDirectoryCommand>(vfs);
  commands["cp"] = std::make_unique<CpCommand>(vfs);
  commands["tree"] = std::make_unique<TreeCommand>(vfs);
  commands["find"] = std::make_unique<FindCommand>(vfs);
}

std::string Parser::processCommand(const std::string &input) {
  std::istringstream stream(input);
  std::string commandName;
  std::vector<std::string> args;

  stream >> commandName;
  std::string arg;
  while (stream >> arg) {
    args.push_back(arg);
  }

  if (commands.find(commandName) != commands.end()) {
    return commands[commandName]->execute(args);
  } else {
    return "Unknown command: " + commandName;
  }
}