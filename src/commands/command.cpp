#include "commands/command.hpp"
#include <iostream>
#include <vector>

ChangeDirectoryCommand::ChangeDirectoryCommand(
    std::shared_ptr<VirtualFilesystem> vfs)
    : vfs(vfs) {}

std::string
ChangeDirectoryCommand::execute(const std::vector<std::string> &args) {
  if (args.size() != 1) {
    return "cd: too many arguments";
  }

  bool success = vfs->changeDirectory(args[0]);
  if (!success) {
    return "cd: no such file or directory: " + args[0];
  }

  return "";
}

ListDirectoryCommand::ListDirectoryCommand(
    std::shared_ptr<VirtualFilesystem> vfs)
    : vfs(vfs) {}

std::string
ListDirectoryCommand::execute(const std::vector<std::string> &args) {
  std::string path = (args.empty()) ? vfs->getCurrentDirectory() : args[0];
  std::string errorMessage;
  auto files = vfs->listDirectory(path, errorMessage);

  if (!errorMessage.empty()) {
    return "ls: cannot access '" + path + "': " + errorMessage;
  }

  if (files.empty()) {
    return "ls: " + path + ": No files found";
  }

  std::string result;
  for (size_t i = 0; i < files.size(); ++i) {
    result += files[i];
    if (i != files.size() - 1)
      result += '\n';
  }
  return result;
}

CpCommand::CpCommand(std::shared_ptr<VirtualFilesystem> vfs)
    : vfs(std::move(vfs)) {}

std::string CpCommand::execute(const std::vector<std::string> &args) {
  if (args.size() != 2) {
    return "cp: too many arguments";
  }

  const std::string &source = args[0];
  const std::string &destination = args[1];

  bool sourceIsDir = false, destinationIsDir = false;

  std::string normalizedSource = vfs->normalizePath(source, sourceIsDir);
  std::string normalizedDestination =
      vfs->normalizePath(destination, destinationIsDir);

  if (!vfs->existsInStorage(normalizedSource)) {
    return "cp: source file or directory does not exist: " + source;
  }

  if (destinationIsDir) {
    normalizedDestination = normalizedDestination + "/" +
                            source.substr(source.find_last_of('/') + 1);
  }

  if (vfs->existsInStorage(normalizedDestination)) {
    return "cp: target already exists: " + destination;
  }

  const Metadata &srcMetadata = vfs->getMetadataFromStorage(normalizedSource);
  if (srcMetadata.fileType == FileType::REG) {
    return copyFile(normalizedSource, normalizedDestination);
  } else if (srcMetadata.fileType == FileType::DIR) {
    return copyDirectory(normalizedSource, normalizedDestination);
  }

  return "cp: unsupported file type";
}

std::string CpCommand::copyFile(const std::string &source,
                                const std::string &destination) {
  size_t size = vfs->getMetadataFromStorage(source).size;
  if (vfs->addFileToArchiveAndStorage(destination, size, FileType::REG)) {
    return "";
  }
  return "cp: failed to copy file: " + source;
}

std::string CpCommand::copyDirectory(const std::string &source,
                                     const std::string &destination) {
  if (!vfs->addFileToArchiveAndStorage(destination, 0, FileType::DIR)) {
    return "cp: failed to create directory: " + destination;
  }

  std::string errorMessage;
  auto files = vfs->listDirectory(source, errorMessage);
  for (const std::string &file : files) {
    std::string srcPath = source + "/" + file;
    std::string destPath = destination + "/" + file;
    const Metadata &metadata = vfs->getMetadataFromStorage(srcPath);
    if (metadata.fileType == FileType::REG) {
      if (!vfs->addFileToArchiveAndStorage(destPath, metadata.size, FileType::REG)) {
        return "cp: failed to copy file: " + file;
      }
    } else if (metadata.fileType == FileType::DIR) {
      if (!vfs->addFileToArchiveAndStorage(destPath, 0, FileType::DIR)) {
        return "cp: failed to copy directory: " + file;
      }
    }
  }

  return "";
}

TreeCommand::TreeCommand(std::shared_ptr<VirtualFilesystem> vfs)
    : vfs(std::move(vfs)) {}

std::string TreeCommand::execute(const std::vector<std::string> &args) {
  std::string directory = args.empty() ? vfs->getCurrentDirectory() : args[0];
  bool isDirectory = false;
  std::string errorMessage;
  return listTree(directory, isDirectory, 0);
}

std::string TreeCommand::listTree(const std::string &path, bool &isDirectory,
                                  int level) {
  std::string result;
  std::string normalizedPath = vfs->normalizePath(path, isDirectory);

  if (!isDirectory) {
    return "";
  }

  std::string errorMessage;
  std::vector<std::string> files = vfs->listDirectory(path, errorMessage);
  for (const std::string &file : files) {
    result += std::string(level * 2, ' ') + file + "\n";
    result += listTree(path + "/" + file, isDirectory, level + 1);
  }

  return result;
}

FindCommand::FindCommand(std::shared_ptr<VirtualFilesystem> vfs)
    : vfs(std::move(vfs)) {}

std::string FindCommand::execute(const std::vector<std::string> &args) {
  if (args.size() != 1) {
    return "find: missing argument";
  }

  std::string searchTerm = args[0];
  return findFiles(vfs->getCurrentDirectory(), searchTerm);
}

std::string FindCommand::findFiles(const std::string &path,
                                   const std::string &searchTerm) {
  bool isDirectory = false;
  std::string output;
  std::string normalizedPath = vfs->normalizePath(path, isDirectory);

  if (!isDirectory) {
    return "";
  }

  std::string errorMessage;
  std::vector<std::string> files =
      vfs->listDirectory(normalizedPath, errorMessage);
  for (const std::string &file : files) {
    std::string fullPath = normalizedPath + "/" + file;

    if (file.find(searchTerm) != std::string::npos) {
      output += fullPath + "\n";
    }

    output += findFiles(fullPath, searchTerm);
  }
  return output;
}