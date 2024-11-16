#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <core/virtual_filesystem.hpp>
#include <string>
#include <vector>

class Command {
public:
  virtual ~Command() = default;
  virtual void execute(const std::vector<std::string> &args) = 0;
};

class ChangeDirectoryCommand : public Command {
public:
  ChangeDirectoryCommand(VirtualFilesystem *vfs);
  void execute(const std::vector<std::string> &args) override;

private:
  VirtualFilesystem *vfs;
};

class ListDirectoryCommand : public Command {
public:
  ListDirectoryCommand(VirtualFilesystem *vfs);
  void execute(const std::vector<std::string> &args) override;

private:
  VirtualFilesystem *vfs;
};

#endif