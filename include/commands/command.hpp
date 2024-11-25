#pragma once
#include <core/virtual_filesystem.hpp>
#include <memory>
#include <string>
#include <vector>

class Command {
public:
  virtual ~Command() = default;
  virtual std::string execute(const std::vector<std::string> &args) = 0;
};

class ChangeDirectoryCommand : public Command {
public:
  ChangeDirectoryCommand(std::shared_ptr<VirtualFilesystem> vfs);
  std::string execute(const std::vector<std::string> &args) override;

private:
  std::shared_ptr<VirtualFilesystem> vfs;
};

class ListDirectoryCommand : public Command {
public:
  ListDirectoryCommand(std::shared_ptr<VirtualFilesystem> vfs);
  std::string execute(const std::vector<std::string> &args) override;

private:
  std::shared_ptr<VirtualFilesystem> vfs;
};

class CpCommand : public Command {
public:
  CpCommand(std::shared_ptr<VirtualFilesystem> vfs);
  std::string execute(const std::vector<std::string> &args) override;
  std::string copyFile(const std::string &source,
                       const std::string &destination);
  std::string copyDirectory(const std::string &source,
                            const std::string &destination);

private:
  std::shared_ptr<VirtualFilesystem> vfs;
};

class TreeCommand : public Command {
public:
  TreeCommand(std::shared_ptr<VirtualFilesystem> vfs);
  std::string execute(const std::vector<std::string> &args) override;
  std::string listTree(const std::string &path, bool &isDirectory, int level);

private:
  std::shared_ptr<VirtualFilesystem> vfs;
};

class FindCommand : public Command {
public:
  FindCommand(std::shared_ptr<VirtualFilesystem> vfs);
  std::string execute(const std::vector<std::string> &args) override;
  std::string findFiles(const std::string &path, const std::string &searchTerm);

private:
  std::shared_ptr<VirtualFilesystem> vfs;
};
