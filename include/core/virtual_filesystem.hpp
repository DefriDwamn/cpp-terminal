#ifndef VIRTUAL_FILESYSTEM_HPP
#define VIRTUAL_FILESYSTEM_HPP
#include "file_storage.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <boost/filesystem.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class VirtualFilesystem {
public:
  VirtualFilesystem(const std::string &path);
  std::vector<std::string> listDirectory(const std::string &path);
  void changeDirectory(const std::string &path);
  bool createFile(const std::string &path, size_t size,
                  unsigned short fileType);
  std::string getCurrentDirectory();

private:
  std::string archivePath;
  std::string currentDirectory;
  std::unique_ptr<FileStorage> fileStorage;
  void loadArchive();
  void createDefaultArchive(const std::string &defaultArchiveName);
};

#endif