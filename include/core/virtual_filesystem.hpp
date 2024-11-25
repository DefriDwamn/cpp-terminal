#pragma once
#include "file_storage.hpp"
#include <boost/filesystem.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class VirtualFilesystem {
public:
  VirtualFilesystem(const std::string &path = "");
  ~VirtualFilesystem();

  std::vector<std::string> listDirectory(const std::string &path,
                                         std::string &errorMessage);
  bool changeDirectory(const std::string &path);
  std::string getCurrentDirectory();
  std::string normalizePath(const std::string &path, bool &isDirectory);

  bool existsInStorage(const std::string &path) const;
  const Metadata &getMetadataFromStorage(const std::string &path) const;
  bool addFileToStorage(const std::string &path, size_t size,
                        FileType fileType);
  bool addFileToArchiveAndStorage(const std::string &path, size_t size,
                                  FileType fileType);

private:
  std::string archivePath;
  std::string currentDirectory;
  struct archive *archive;
  std::unique_ptr<FileStorage> fileStorage;

  void loadArchive();
  void createDefaultArchive();
  void addFileToArchive(const std::string &path, size_t size,
                        FileType fileType);
};
