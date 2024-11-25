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
  if (!archivePath.empty()) {
    loadArchive();
  } else {
    archivePath = "fs.tar";
    createDefaultArchive();
  }
}

void VirtualFilesystem::createDefaultArchive() {
  if (std::filesystem::exists(archivePath)) {
    throw std::runtime_error("Archive already exists: " + archivePath);
  }

  struct archive *archive = archive_write_new();
  if (archive == nullptr) {
    throw std::runtime_error("Failed to create archive");
  }

  if (archive_write_set_format_pax_restricted(archive) != ARCHIVE_OK) {
    archive_write_free(archive);
    throw std::runtime_error("Failed to set archive format: " +
                             std::string(archive_error_string(archive)));
  }

  if (archive_write_open_filename(archive, archivePath.c_str()) != ARCHIVE_OK) {
    archive_write_free(archive);
    throw std::runtime_error("Failed to open archive: " +
                             std::string(archive_error_string(archive)));
  }

  addFileToArchive(archive, "/hello", std::strlen("Hello, world!"),
                   FileType::REG);
  addFile("/hello", std::strlen("Hello, world!"), FileType::REG);

  addFileToArchive(archive, "/dir", 0, FileType::DIR);
  addFile("/dir", 0, FileType::DIR);

  addFileToArchive(archive, "/dir/dir2", 0, FileType::DIR);
  addFile("/dir/dir2", 0, FileType::DIR);

  addFileToArchive(archive, "/dir/file", 0, FileType::REG);
  addFile("/dir/file", 0, FileType::REG);

  if (archive_write_close(archive) != ARCHIVE_OK) {
    archive_write_free(archive);
    throw std::runtime_error("Failed to close archive: " +
                             std::string(archive_error_string(archive)));
  }

  archive_write_free(archive);
}

void VirtualFilesystem::loadArchive() {
  struct archive *archive = archive_read_new();
  if (archive == nullptr) {
    throw std::runtime_error("Failed to create archive reader");
  }

  if (archive_read_support_format_tar(archive) != ARCHIVE_OK) {
    archive_read_free(archive);
    throw std::runtime_error("Failed to set archive format: " +
                             std::string(archive_error_string(archive)));
  }

  if (archive_read_open_filename(archive, archivePath.c_str(), 10240) !=
      ARCHIVE_OK) {
    archive_read_free(archive);
    throw std::runtime_error("Failed to open archive: " +
                             std::string(archive_error_string(archive)));
  }

  struct archive_entry *entry;
  while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
    const char *path = archive_entry_pathname(entry);
    size_t size = archive_entry_size(entry);
    int type = archive_entry_filetype(entry);
    addFile(path, size, (type == AE_IFDIR) ? FileType::DIR : FileType::REG);
  }

  if (archive_read_free(archive) != ARCHIVE_OK) {
    throw std::runtime_error("Failed to free archive: " +
                             std::string(archive_error_string(archive)));
  }
}

void VirtualFilesystem::addFileToArchive(struct archive *archive,
                                         const std::string &path, size_t size,
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
    throw std::runtime_error("Failed to write header for " + path + ": " +
                             std::string(archive_error_string(archive)));
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

bool VirtualFilesystem::addFile(const std::string &path, size_t size,
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
    if (fileStorage->exists(path)) {
        std::cerr << "File or directory already exists in storage: " << path << std::endl;
        return false;
    }

    fileStorage->add(path, size, fileType);

    struct archive *archive = archive_read_new();
    if (archive == nullptr) {
        std::cerr << "Failed to create archive object" << std::endl;
        return false;
    }

    if (archive_read_support_format_tar(archive) != ARCHIVE_OK) {
        archive_read_free(archive);
        std::cerr << "Failed to set archive format" << std::endl;
        return false;
    }

    if (archive_read_open_filename(archive, archivePath.c_str(), 10240) != ARCHIVE_OK) {
        archive_read_free(archive);
        std::cerr << "Failed to open archive for reading" << std::endl;
        return false;
    }

    std::vector<struct archive_entry*> entries;
    struct archive_entry *entry;
    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
        struct archive_entry *entry_copy = archive_entry_clone(entry);
        if (entry_copy == nullptr) {
            std::cerr << "Failed to create archive entry copy" << std::endl;
            archive_read_free(archive);
            return false;
        }
        entries.push_back(entry_copy);
    }

    if (archive_read_free(archive) != ARCHIVE_OK) {
        std::cerr << "Failed to free archive" << std::endl;
        return false;
    }

    struct archive *archive_write = archive_write_new();
    if (archive_write == nullptr) {
        std::cerr << "Failed to create archive write object" << std::endl;
        return false;
    }

    if (archive_write_set_format_pax_restricted(archive_write) != ARCHIVE_OK) {
        archive_write_free(archive_write);
        std::cerr << "Failed to set archive format for writing" << std::endl;
        return false;
    }

    if (archive_write_open_filename(archive_write, archivePath.c_str()) != ARCHIVE_OK) {
        archive_write_free(archive_write);
        std::cerr << "Failed to open archive for writing" << std::endl;
        return false;
    }

    for (auto &entry_copy : entries) {
        if (archive_write_header(archive_write, entry_copy) != ARCHIVE_OK) {
            archive_entry_free(entry_copy);
            archive_write_free(archive_write);
            std::cerr << "Failed to write entry header to archive" << std::endl;
            return false;
        }

        if (archive_entry_filetype(entry_copy) == AE_IFREG) {
            const char *content = "Hello, world!";
            long content_size = std::strlen(content); 
            std::cout << "Writing " << content_size << " bytes of data to archive..." << std::endl;
            long written = archive_write_data(archive_write, content, content_size);

            if (written != content_size) {
                std::cerr << "Failed to write data to archive. Written: " << written << " bytes, Expected: " << content_size << " bytes." << std::endl;
                archive_entry_free(entry_copy);
                archive_write_free(archive_write);
                return false;
            }
        }

        archive_entry_free(entry_copy);
    }

    addFileToArchive(archive_write, path, size, fileType);

    if (archive_write_close(archive_write) != ARCHIVE_OK) {
        archive_write_free(archive_write);
        std::cerr << "Failed to close the archive after writing" << std::endl;
        return false;
    }

    archive_write_free(archive_write);
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