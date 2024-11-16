#include "core/parser.hpp"
#include <iostream>
#include <sstream>

Parser::Parser(VirtualFilesystem *vfs) {
  commands["cd"] = std::make_unique<ChangeDirectoryCommand>(vfs);
  commands["ls"] = std::make_unique<ListDirectoryCommand>(vfs);
}

void Parser::processCommand(const std::string &input) {
  std::istringstream stream(input);
  std::string commandName;
  std::vector<std::string> args;

  stream >> commandName;
  std::string arg;
  while (stream >> arg) {
    args.push_back(arg);
  }

  if (commands.find(commandName) != commands.end()) {
    commands[commandName]->execute(args);
  } else {
    std::cerr << "Unknown command: " << commandName << std::endl;
  }
}