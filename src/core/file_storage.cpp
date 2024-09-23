#include "core/file_storage.hpp"

void FileStorage::add(const std::string &path, size_t size,
                      unsigned short fileType) {
  if (files.find(path) != files.end()) {
    std::cerr << "File or directory already exists: " << path << std::endl;
    return;
  }
  files[path] = Metadata(path, size, fileType);
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