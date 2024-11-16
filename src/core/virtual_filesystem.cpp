#include "core/virtual_filesystem.hpp"
#include "core/file_storage.hpp"
#include <QArchive/QArchive>
#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <iostream>
#include <memory>
#include <sstream>

VirtualFilesystem::VirtualFilesystem(const std::string &path)
    : currentDirectory("/"), archivePath(path) {
  fileStorage = std::make_unique<FileStorage>();
  if (!archivePath.empty()) {
    loadArchive();
  } else {
    createDefaultArchive("fs.tar");
    loadArchive();
    addFile("/hello", 0, FileType::REG);
    addFile("/dir", 0, FileType::DIR);
    addFile("/dir/file", 0, FileType::REG);
  }
}

void VirtualFilesystem::createDefaultArchive(
    const std::string &defaultArchiveName) {
  QFile archiveFile(QString::fromStdString(defaultArchiveName));

  if (archiveFile.exists()) {
    throw std::runtime_error("Default archive already exists");
  }

  QArchive::DiskCompressor compressor(
      QString::fromStdString(defaultArchiveName));
  compressor.setArchiveFormat(QArchive::TarFormat);

  QObject::connect(&compressor, &QArchive::DiskCompressor::finished,
                   [&]() { archivePath = defaultArchiveName; });

  compressor.start();
}

void VirtualFilesystem::loadArchive() {
  QFile archiveFile(QString::fromStdString(archivePath));
  archiveFile.open(QIODevice::ReadOnly);

  if (!archiveFile.exists()) {
    throw std::runtime_error("Archive file does not exist: " + archivePath);
  }

  QArchive::MemoryExtractor extractor(&archiveFile);

  QObject::connect(
      &extractor, &QArchive::MemoryExtractor::finished,
      [&](QArchive::MemoryExtractorOutput *output) {
        auto files = output->getFiles();
        std::cout << "feaw";
        for (const auto &file : files) {
          auto fileInfo = file.fileInformation();

          auto path = fileInfo.value("FileName").toString().toStdString();
          auto type = fileInfo.value("FileType").toString().toStdString();
          auto size = fileInfo.value("Size").toString().toStdString();
          std::cout << path << '\n';

          fileStorage->add(path, std::stoi(size),
                           type == "Dir" ? FileType::DIR : FileType::REG);
        }

        output->deleteLater();
        return;
      });

  QObject::connect(&extractor, &QArchive::MemoryExtractor::started, [&]() {
    std::cout << "HELLO" << '\n';
    return;
  });

  QObject::connect(&extractor, &QArchive::MemoryExtractor::error,
                   [&](short code) {
                     qInfo() << "An error has occured ::"
                             << QArchive::errorCodeToString(code);
                     return;
                   });

  extractor.start();
}

bool VirtualFilesystem::addFile(const std::string &path, size_t size,
                                FileType fileType) {
  if (fileStorage->exists(path)) {
    std::cerr << "File or directory already exists: " << path << std::endl;
    return false;
  }

  QArchive::MemoryCompressor compressor(QArchive::TarFormat);
  compressor.addFiles(QString::fromStdString(path));

  QObject::connect(&compressor, &QArchive::MemoryCompressor::finished,
                   [&](QBuffer *buffer) {
                     QFile archiveFile(QString::fromStdString(archivePath));
                     if (archiveFile.open(QIODevice::WriteOnly)) {
                       archiveFile.write(buffer->data());
                       archiveFile.close();
                     }
                     buffer->deleteLater();
                   });

  QObject::connect(&compressor, &QArchive::MemoryCompressor::error,
                   [&](short code, QString file) {
                     qInfo()
                         << "An error has occured ::"
                         << QArchive::errorCodeToString(code) << "::" << file;
                     return;
                   });

  compressor.start();

  return true;
}

std::string VirtualFilesystem::getCurrentDirectory() {
  return currentDirectory;
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
  for (const auto &seg : pathSegments) {
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
  if (metadata.fileType != FileType::DIR) {
    std::cerr << "Path is not a directory: " << normalizedPath << std::endl;
    return;
  }

  currentDirectory = normalizedPath;
}

std::vector<std::string>
VirtualFilesystem::listDirectory(const std::string &path) {
  std::vector<std::string> result;

  if (!fileStorage->exists(path)) {
    std::cerr << "Directory does not exist: " << path << std::endl;
    return result;
  }

  const Metadata &metadata = fileStorage->getMetadata(path);
  if (metadata.fileType != FileType::DIR) {
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