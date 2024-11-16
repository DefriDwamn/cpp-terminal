#include "core/shell.hpp"
#include "core/parser.hpp"
#include "core/virtual_filesystem.hpp"
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QTextStream>
#include <iostream>
#include <memory>

Shell::Shell() {
  vfs = std::make_unique<VirtualFilesystem>("");
  parser = std::make_unique<Parser>(vfs.get());
  running = true;
}

Shell::Shell(const std::string &fsPath) {
  vfs = std::make_unique<VirtualFilesystem>(fsPath);
  parser = std::make_unique<Parser>(vfs.get());
  running = true;
}

void Shell::handleInput() {
  QTextStream inputStream(stdin);
  QString input = inputStream.readLine();

  if (input.trimmed() == "exit") {
    running = false;
    return;
  }

  parser->processCommand(input.toStdString());
  std::cout << "> ";
  std::cout.flush();
}

void Shell::run() {
  std::cout << "Type 'exit' to quit." << std::endl;
  std::cout << "> ";
  std::cout.flush();
  while (running) {
    
    QCoreApplication::processEvents();
    handleInput();
  }
}