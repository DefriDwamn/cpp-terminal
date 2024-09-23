#ifndef SHELL_HPP
#define SHELL_HPP
#include "commands/command.hpp"
#include "core/parser.hpp"
#include "virtual_filesystem.hpp"
#include <iostream>
#include <memory>
#include <string>

class Shell {
public:
  Shell();
  Shell(const std::string &fsPath);
  void run();

private:
  std::unique_ptr<VirtualFilesystem> vfs;
  std::unique_ptr<Parser> parser;
};

#endif