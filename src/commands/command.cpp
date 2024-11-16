#include "commands/command.hpp"
#include <iostream>

ChangeDirectoryCommand::ChangeDirectoryCommand(VirtualFilesystem *vfs)
    : vfs(vfs) {}

void ChangeDirectoryCommand::execute(const std::vector<std::string> &args) {
  if (args.size() != 1) {
    std::cerr << "Usage: cd <directory>" << std::endl;
    return;
  }
  vfs->changeDirectory(args[0]);
}

ListDirectoryCommand::ListDirectoryCommand(VirtualFilesystem *vfs) : vfs(vfs) {}

void ListDirectoryCommand::execute(const std::vector<std::string> &args) {
  std::string path = (args.empty()) ? vfs->getCurrentDirectory() : args[0];
  auto files = vfs->listDirectory(path);
  for (const auto &file : files) {
    std::cout << file << ' ';
  }
  std::cout << '\n';
}
