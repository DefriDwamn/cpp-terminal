#include "core/virtual_filesystem.hpp"
#include <sstream>

VirtualFilesystem::VirtualFilesystem(const std::string &path)
    : currentDirectory("/"), archivePath(path) {
    fileStorage = std::make_unique<FileStorage>();
    if (!archivePath.empty()) {
        loadArchive();
    } else {
        createDefaultArchive("fs.tar");
        loadArchive();
        createFile("/hello", 1, AE_IFREG);
        createFile("/dir", 0, AE_IFDIR);
        createFile("/dir/file", 0, AE_IFREG);
    }
}

std::string VirtualFilesystem::getCurrentDirectory() {
    return currentDirectory;
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

        std::string relativePath = path[0] == '/' ? std::string(path) : "/" + std::string(path);

        fileStorage->add(relativePath, fileSize, fileMode);
        std::cout << "Loaded: " << relativePath << " [" << fileMode << "]" << std::endl;

        archive_read_data_skip(archive);
    }

    result = archive_read_free(archive);
    if (result != ARCHIVE_OK) {
        throw std::runtime_error("Failed to close archive: " + archivePath);
    }
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
    for (const auto& seg : pathSegments) {
        normalizedPath += seg + "/";
    }
    if (normalizedPath.size() > 1) {
        normalizedPath.pop_back();
    }

    if (!fileStorage->exists(normalizedPath)) {
        std::cerr << "Directory does not exist: " << normalizedPath << std::endl;
        return;
    }

    const Metadata &metadata = fileStorage->getMetadata(normalizedPath);
    if (metadata.fileType != AE_IFDIR) {
        std::cerr << "Path is not a directory: " << normalizedPath << std::endl;
        return;
    }

    currentDirectory = normalizedPath;
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

bool VirtualFilesystem::createFile(const std::string &path, size_t size, unsigned short fileType) {
    if (fileStorage->exists(path)) {
        std::cerr << "File or directory already exists: " << path << std::endl;
        return false;
    }

    struct archive *archive = archive_write_new();
    archive_write_set_format_pax_restricted(archive);

    // Открываем архив для добавления нового файла или директории
    if (archive_write_open_filename(archive, archivePath.c_str()) != ARCHIVE_OK) {
        std::cerr << "Failed to open archive for writing: " << archive_error_string(archive) << std::endl;
        archive_write_free(archive);
        return false;
    }

    struct archive_entry *entry = archive_entry_new();
    // Убираем ведущий слэш, если он есть
    std::string relativePath = path;
    if (!relativePath.empty() && relativePath[0] == '/') {
        relativePath = relativePath.substr(1);
    }
    archive_entry_set_pathname(entry, relativePath.c_str());
    archive_entry_set_size(entry, size);
    archive_entry_set_filetype(entry, fileType);
    archive_entry_set_perm(entry, 0755);


    std::cout << archive_entry_pathname_utf8(entry) << '\n';

    // Записываем заголовок для нового файла или директории
    if (archive_write_header(archive, entry) != ARCHIVE_OK) {
        std::cerr << "Failed to write header: " << archive_error_string(archive) << std::endl;
        archive_entry_free(entry);
        archive_write_free(archive);
        return false;
    }

    // Если это обычный файл, записываем пустые данные
    if (fileType == AE_IFREG && size > 0) {
        std::vector<char> buffer(size, 0);
        if (archive_write_data(archive, buffer.data(), size) != static_cast<ssize_t>(size)) {
            std::cerr << "Failed to write file data: " << archive_error_string(archive) << std::endl;
            archive_entry_free(entry);
            archive_write_free(archive);
            return false;
        }
    }

    archive_entry_free(entry);
    archive_write_free(archive);

    // Добавляем информацию о файле в fileStorage для отслеживания
    fileStorage->add(path, size, fileType);

    return true;
}

