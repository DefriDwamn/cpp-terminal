#ifndef FILE_STORAGE_HPP
#define FILE_STORAGE_HPP
#include <archive_entry.h>
#include <iostream>
#include <string>
#include <unordered_map>

struct Metadata {
  std::string path;
  size_t size;
  unsigned short fileType;

  Metadata(const std::string &path, size_t size, unsigned short type)
      : path(path), size(size), fileType(type) {}
  Metadata() : path(""), size(0), fileType(0) {}
};

class FileStorage {
public:
  std::unordered_map<std::string, Metadata> files;
  void add(const std::string &path, size_t size, unsigned short fileType);
  bool remove(const std::string &path);
  bool exists(const std::string &path) const;
  const Metadata &getMetadata(const std::string &path) const;
  FileStorage();
};
#endif