#include "core/virtual_filesystem.hpp"
#include "core/file_storage.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

VirtualFilesystem::VirtualFilesystem(const std::string &path)
    : currentDirectory("/"), archivePath(path) {
  fileStorage = std::make_unique<FileStorage>();
  archive = archive_write_new();
  if (archive == nullptr)
    throw std::runtime_error("Failed to create archive");

  if (!archivePath.empty()) {
    loadArchive();
  } else {
    archivePath = "fs.tar";
    createDefaultArchive();
  }
}

VirtualFilesystem::~VirtualFilesystem() {
  if (archive != nullptr) {
    archive_write_close(archive);
    archive_write_free(archive);
  }
}

void VirtualFilesystem::createDefaultArchive() {
  if (std::filesystem::exists(archivePath)) {
    throw std::runtime_error("Archive already exists: " + archivePath);
  }

  if (archive_write_set_format_pax_restricted(archive) != ARCHIVE_OK) {
    archive_write_free(archive);
    throw std::runtime_error("Failed to set archive format");
  }
  if (archive_write_open_filename(archive, archivePath.c_str()) != ARCHIVE_OK) {
    throw std::runtime_error("Failed to open archive for writing");
  }

  addFileToArchiveAndStorage("/hello", std::strlen("Hello, world!"),
                             FileType::REG);
  addFileToArchiveAndStorage("/dir", 0, FileType::DIR);
  addFileToArchiveAndStorage("/dir/dir2", 0, FileType::DIR);
  addFileToArchiveAndStorage("/dir/file", 0, FileType::REG);
}

void VirtualFilesystem::loadArchive() {
  std::string tempPath = archivePath + ".temp";
  struct archive *tempArchive = archive_write_new();
  if (archive_write_set_format_pax_restricted(tempArchive) != ARCHIVE_OK) {
    throw std::runtime_error("Failed to set format for temp archive");
  }
  if (archive_write_open_filename(tempArchive, tempPath.c_str()) !=
      ARCHIVE_OK) {
    archive_write_free(tempArchive);
    throw std::runtime_error("Failed to open temp archive for writing");
  }

  struct archive *archiveReader = archive_read_new();
  if (archiveReader == nullptr) {
    archive_write_free(tempArchive);
    throw std::runtime_error("Failed to create archive reader");
  }
  if (archive_read_support_format_tar(archiveReader) != ARCHIVE_OK) {
    archive_write_free(tempArchive);
    archive_read_free(archiveReader);
    throw std::runtime_error("Failed to set archive format for reading");
  }
  if (archive_read_open_filename(archiveReader, archivePath.c_str(), 10240) !=
      ARCHIVE_OK) {
    archive_write_free(tempArchive);
    archive_read_free(archiveReader);
    throw std::runtime_error("Failed to open archive for reading");
  }

  struct archive_entry *entry;
  while (archive_read_next_header(archiveReader, &entry) == ARCHIVE_OK) {
    const char *path = archive_entry_pathname(entry);
    size_t size = archive_entry_size(entry);
    int type = archive_entry_filetype(entry);
    FileType fileType = (type == AE_IFDIR) ? FileType::DIR : FileType::REG;

    addFileToStorage(path, size, fileType);

    archive_write_header(tempArchive, entry);
    if (fileType == FileType::REG && size > 0) {
      std::vector<char> buffer(size);
      archive_read_data(archiveReader, buffer.data(), size);
      archive_write_data(tempArchive, buffer.data(), size);
    }
  }

  archive_read_free(archiveReader);
  archive_write_close(tempArchive);
  archive_write_free(tempArchive);

  if (archive_write_set_format_pax_restricted(archive) != ARCHIVE_OK) {
    throw std::runtime_error("Failed to set archive format");
  }
  if (archive_write_open_filename(archive, archivePath.c_str()) != ARCHIVE_OK) {
    throw std::runtime_error("Failed to open archive for writing");
  }

  struct archive *tempReader = archive_read_new();
  if (archive_read_support_format_tar(tempReader) != ARCHIVE_OK) {
    archive_read_free(tempReader);
    throw std::runtime_error("Failed to set format for temp reading");
  }
  if (archive_read_open_filename(tempReader, tempPath.c_str(), 10240) !=
      ARCHIVE_OK) {
    archive_read_free(tempReader);
    throw std::runtime_error("Failed to open temp archive for reading");
  }

  while (archive_read_next_header(tempReader, &entry) == ARCHIVE_OK) {
    archive_write_header(archive, entry);
    size_t size = archive_entry_size(entry);
    if (archive_entry_filetype(entry) == AE_IFREG && size > 0) {
      std::vector<char> buffer(size);
      archive_read_data(tempReader, buffer.data(), size);
      archive_write_data(archive, buffer.data(), size);
    }
  }

  archive_read_free(tempReader);
  std::filesystem::remove(tempPath);
}

void VirtualFilesystem::addFileToArchive(const std::string &path, size_t size,
                                         FileType fileType) {
  struct archive_entry *entry = archive_entry_new();
  if (entry == nullptr) {
    throw std::runtime_error("Failed to create archive entry");
  }

  archive_entry_set_pathname(entry, path.c_str());
  archive_entry_set_size(entry, size);

  int archiveType = (fileType == FileType::DIR) ? AE_IFDIR : AE_IFREG;
  archive_entry_set_filetype(entry, archiveType);
  archive_entry_set_perm(entry, 0755);

  if (archive_write_header(archive, entry) != ARCHIVE_OK) {
    archive_entry_free(entry);
    throw std::runtime_error("Failed to write header for " + path);
  }

  if (fileType == FileType::REG && size > 0) {
    const char *content = "Hello, world!";
    if (archive_write_data(archive, content, std::strlen(content)) !=
        std::strlen(content)) {
      archive_entry_free(entry);
      throw std::runtime_error("Failed to write data to " + path);
    }
  }

  archive_entry_free(entry);
}

bool VirtualFilesystem::addFileToStorage(const std::string &path, size_t size,
                                         FileType fileType) {
  if (fileStorage->exists(path)) {
    std::cerr << "File or directory already exists: " << path << std::endl;
    return false;
  }

  fileStorage->add(path, size, fileType);
  return true;
}

bool VirtualFilesystem::addFileToArchiveAndStorage(const std::string &path,
                                                   size_t size,
                                                   FileType fileType) {
  if (!addFileToStorage(path, size, fileType)) {
    return false;
  }

  try {
    addFileToArchive(path, size, fileType);
  } catch (const std::exception &e) {
    std::cerr << "Error adding file to archive: " << e.what() << std::endl;
    return false;
  }

  return true;
}

std::string VirtualFilesystem::getCurrentDirectory() {
  return currentDirectory;
}

std::string VirtualFilesystem::normalizePath(const std::string &path,
                                             bool &isDirectory) {
  std::string targetPath = path;

  if (targetPath.empty() || targetPath == ".") {
    targetPath = currentDirectory;
  } else if (targetPath[0] != '/') {
    if (currentDirectory.back() != '/') {
      targetPath = currentDirectory + "/" + targetPath;
    } else {
      targetPath = currentDirectory + targetPath;
    }
  }

  if (targetPath == "..") {
    if (currentDirectory == "/") {
      targetPath = currentDirectory;
    } else {
      size_t lastSlashPos = currentDirectory.find_last_of('/');
      targetPath = currentDirectory.substr(0, lastSlashPos);
    }
  }

  std::vector<std::string> pathSegments;
  std::stringstream ss(targetPath);
  std::string segment;

  while (std::getline(ss, segment, '/')) {
    if (segment == "..") {
      if (!pathSegments.empty()) {
        pathSegments.pop_back();
      }
    } else if (!segment.empty() && segment != ".") {
      pathSegments.push_back(segment);
    }
  }

  std::string normalizedPath = "/";
  for (const auto &seg : pathSegments) {
    normalizedPath += seg + "/";
  }

  if (normalizedPath.size() > 1) {
    normalizedPath.pop_back();
  }

  bool pathExists = fileStorage->exists(normalizedPath);
  if (!pathExists) {
    isDirectory = false;
    return normalizedPath;
  }

  const Metadata &metadata = fileStorage->getMetadata(normalizedPath);
  isDirectory = (metadata.fileType == FileType::DIR);

  return normalizedPath;
}

bool VirtualFilesystem::changeDirectory(const std::string &path) {
  bool isDirectory = false;
  std::string targetPath = normalizePath(path, isDirectory);

  if (!isDirectory) {
    std::cerr << "Not a valid directory: " << targetPath << std::endl;
    return false;
  }

  currentDirectory = targetPath;
  return true;
}

std::vector<std::string>
VirtualFilesystem::listDirectory(const std::string &path,
                                 std::string &errorMessage) {
  std::vector<std::string> result;

  bool isDirectory = false;
  std::string directoryPath = normalizePath(path, isDirectory);

  if (!isDirectory) {
    errorMessage = "Directory does not exist";
    return result;
  }

  if (directoryPath.back() != '/') {
    directoryPath += '/';
  }

  for (const auto &file : fileStorage->files) {
    const std::string &filePath = file.second.path;

    if (filePath.find(directoryPath) == 0) {
      std::string relativePath = filePath.substr(directoryPath.size());

      if (!relativePath.empty() &&
          relativePath.find('/') == std::string::npos) {
        result.push_back(relativePath);
      } else if (!relativePath.empty() && relativePath.back() == '/' &&
                 relativePath.find('/', 0) == relativePath.size() - 1) {
        result.push_back(relativePath.substr(0, relativePath.size() - 1));
      }
    }
  }

  return result;
}

bool VirtualFilesystem::existsInStorage(const std::string &path) const {
  return fileStorage->exists(path);
}

const Metadata &
VirtualFilesystem::getMetadataFromStorage(const std::string &path) const {
  return fileStorage->getMetadata(path);
}