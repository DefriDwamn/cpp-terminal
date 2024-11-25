#include "core/file_storage.hpp"
#include <iostream>

FileStorage::FileStorage() { add("/", 0, FileType::DIR); }

void FileStorage::add(const std::string &path, size_t size, FileType fileType) {
  if (path.empty()) {
    std::cerr << "Error: Path cannot be empty.\n";
    return;
  }

  std::string adjustedPath = path;
  if (fileType == FileType::DIR && path.back() == '/' && path.size() != 1) {
    adjustedPath.pop_back();
  }

  if (files.find(adjustedPath) != files.end()) {
    std::cerr << "Error: File or directory already exists: " + adjustedPath
              << '\n';
    return;
  }

  files[adjustedPath] = Metadata(adjustedPath, size, fileType);
}

bool FileStorage::remove(const std::string &path) {
  if (files.erase(path) == 1) {
    return true;
  } else {
    std::cerr << "File or directory not found: " << path << std::endl;
    return false;
  }
}

bool FileStorage::exists(const std::string &path) const {
  return files.find(path) != files.end();
}

const Metadata &FileStorage::getMetadata(const std::string &path) const {
  auto it = files.find(path);
  if (it == files.end()) {
    throw std::runtime_error("File or directory not found: " + path);
  }
  return it->second;
}
