#include "core/virtual_filesystem.hpp"
#include <QArchive>
#include <QBuffer>
#include <QFile>
#include <QCoreApplication>
#include <iostream>
#include <memory>

VirtualFilesystem::VirtualFilesystem(const std::string &path)
    : currentDirectory("/"), archivePath(path) {
  fileStorage = std::make_unique<FileStorage>();
  if (!archivePath.empty()) {
    loadArchive();
  } else {
    createDefaultArchive("fs.tar");
    loadArchive();
    createFile("/hello", 0, AE_IFREG);
    createFile("/dir", 0, AE_IFDIR);
    createFile("/dir/file", 0, AE_IFREG);
  }
}

void VirtualFilesystem::createDefaultArchive(const std::string &defaultArchiveName) {
    QFile archiveFile(QString::fromStdString(defaultArchiveName));
    if (archiveFile.exists()) {
        throw std::runtime_error("Default archive already exists");
    }

    QArchive::MemoryCompressor compressor(QArchive::TarFormat);

    QObject::connect(&compressor, &QArchive::MemoryCompressor::finished, [&](QBuffer *buffer) {
        if (archiveFile.open(QIODevice::WriteOnly)) {
            archiveFile.write(buffer->data());
            archiveFile.close();
        }
        buffer->deleteLater();
        archivePath = defaultArchiveName;
    });

    compressor.start();
}

void VirtualFilesystem::loadArchive() {
    QFile archiveFile(QString::fromStdString(archivePath));
    if (!archiveFile.exists()) {
        throw std::runtime_error("Archive file does not exist: " + archivePath);
    }

    QArchive::MemoryExtractor extractor;
    extractor.setArchive(QIODevice::ReadOnly, &archiveFile);

    QObject::connect(&extractor, &QArchive::MemoryExtractor::finished, [&](QBuffer *buffer) {
        buffer->deleteLater();
    });

    extractor.start();
}

bool VirtualFilesystem::createFile(const std::string &path, size_t size, unsigned short fileType) {
    if (fileStorage->exists(path)) {
        std::cerr << "File or directory already exists: " << path << std::endl;
        return false;
    }

    QArchive::MemoryCompressor compressor(QArchive::TarFormat);
    compressor.addFiles(QString::fromStdString(path));

    QObject::connect(&compressor, &QArchive::MemoryCompressor::finished, [&](QBuffer *buffer) {
        QFile archiveFile(QString::fromStdString(archivePath));
        if (archiveFile.open(QIODevice::WriteOnly)) {
            archiveFile.write(buffer->data());
            archiveFile.close();
        }
        buffer->deleteLater();
    });

    compressor.start();

    fileStorage->add(path, size, fileType);
    return true;
}
