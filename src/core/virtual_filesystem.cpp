#include "core/virtual_filesystem.hpp"

VirtualFilesystem::VirtualFilesystem(const std::string &path)
    : currentDirectory("/"), archivePath(path) {
  fileStorage = std::make_unique<FileStorage>();
  if (!archivePath.empty()) {
    loadArchive();
  } else {
    createDefaultArchive("fs.tar");
    loadArchive();
    createFile("/", 0, AE_IFDIR);
    createFile("/hello", 0, AE_IFREG);
    createFile("/dir", 0, AE_IFDIR);
    createFile("/dir/file", 0, AE_IFREG);
  }
}

std::string VirtualFilesystem::getCurrentDirectory() {
  return currentDirectory;
}

void VirtualFilesystem::loadArchive() {
    struct archive *archive;
    struct archive_entry *entry;
    int result;

    archive = archive_read_new();
    archive_read_support_format_tar(archive);
    archive_read_support_filter_all(archive);

    result = archive_read_open_filename(archive, archivePath.c_str(), 10240);
    if (result != ARCHIVE_OK)
        throw std::runtime_error("Failed to open archive!");

    while (archive_read_next_header(archive, &entry) == ARCHIVE_OK) {
        const char *path = archive_entry_pathname(entry);
        unsigned short fileMode = archive_entry_filetype(entry);
        size_t fileSize = archive_entry_size(entry);

        std::string relativePath = path[0] == '/' ? std::string(path).substr(1) : std::string(path);

        fileStorage->add(relativePath, fileSize, fileMode);

        archive_read_data_skip(archive);
    }

    result = archive_read_free(archive);
    if (result != ARCHIVE_OK) {
        throw std::runtime_error("Failed to close archive: " + archivePath);
    }
}

void VirtualFilesystem::createDefaultArchive(
    const std::string &defaultArchiveName) {
  if (boost::filesystem::exists(defaultArchiveName)) {
    throw std::runtime_error("Default archive already exists");
  }

  struct archive *a = archive_write_new();
  archive_write_set_format_pax_restricted(a);

  if (archive_write_open_filename(a, defaultArchiveName.c_str()) !=
      ARCHIVE_OK) {
    std::cerr << "Failed to create archive: " << archive_error_string(a)
              << std::endl;
    archive_write_free(a);
    return;
  }
  archive_write_free(a);

  archivePath = defaultArchiveName;
}

std::vector<std::string> VirtualFilesystem::listDirectory(const std::string &path) {
  std::vector<std::string> result;

  if (!fileStorage->exists(path)) {
    std::cerr << "Directory does not exist: " << path << std::endl;
    return result;
  }

  const Metadata &metadata = fileStorage->getMetadata(path);
  if (metadata.fileType != AE_IFDIR) {
    std::cerr << "Path is not a directory: " << path << std::endl;
    return result;
  }

  std::string directoryPath = path;
  if (directoryPath.back() != '/') {
    directoryPath += "/";
  }

  for (const auto &file : fileStorage->files) {
    const std::string &filePath = file.second.path;
    if (filePath.find(directoryPath) == 0) {
      std::string relativePath = filePath.substr(directoryPath.size());
      if (relativePath.find('/') == std::string::npos) {
        result.push_back(relativePath);
      }
    }
  }

  return result;
}

void VirtualFilesystem::changeDirectory(const std::string &path) {
    std::string targetPath = path;

    if (targetPath.empty() || targetPath[0] != '/') {
        if (currentDirectory.back() != '/') {
            targetPath = currentDirectory + "/" + targetPath;
        } else {
            targetPath = currentDirectory + targetPath;
        }
    }


    if (!fileStorage->exists(targetPath)) {
        std::cerr << "Directory does not exist: " << targetPath << std::endl;
        return;
    }

    const Metadata &metadata = fileStorage->getMetadata(targetPath);
    if (metadata.fileType != AE_IFDIR) {
        std::cerr << "Path is not a directory: " << targetPath << std::endl;
        return;
    }

    currentDirectory = targetPath;
}



bool VirtualFilesystem::createFile(const std::string &path, size_t size,
                                   unsigned short fileType) {
  if (fileStorage->exists(path)) {
    return false;
  }
  struct archive *a = archive_write_new();
  archive_write_set_format_ustar(a);

  if (archive_write_open_filename(a, archivePath.c_str()) != ARCHIVE_OK) {
    std::cerr << "Failed to open archive for writing: "
              << archive_error_string(a) << std::endl;
    archive_write_free(a);
    return false;
  }

  struct archive_entry *entry = archive_entry_new();
  archive_entry_set_pathname(entry, path.c_str());
  archive_entry_set_size(entry, size);
  archive_entry_set_filetype(entry, fileType);
  archive_entry_set_perm(entry, 0644);

  if (archive_write_header(a, entry) != ARCHIVE_OK) {
    std::cerr << "Failed to write header: " << archive_error_string(a)
              << std::endl;
    archive_entry_free(entry);
    archive_write_free(a);
    return false;
  }

  archive_entry_free(entry);
  archive_write_free(a);

  fileStorage->add(path, size, fileType);

  return true;
}
