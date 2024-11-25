#pragma once
#include <string>
#include <unordered_map>

enum FileType { REG, DIR };

struct Metadata {
  std::string path;
  size_t size;
  FileType fileType;

  Metadata(const std::string &path, size_t size, FileType fileType)
      : path(path), size(size), fileType(fileType) {}
  Metadata() : path(""), size(0), fileType(FileType::REG) {}
};

class FileStorage {
public:
  std::unordered_map<std::string, Metadata> files;
  void add(const std::string &path, size_t size, FileType fileType);
  bool remove(const std::string &path);
  bool exists(const std::string &path) const;
  const Metadata &getMetadata(const std::string &path) const;
  FileStorage();
};
