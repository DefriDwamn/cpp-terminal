#ifndef SHELL_HPP
#define SHELL_HPP
#include "commands/command.hpp"
#include "core/parser.hpp"
#include "virtual_filesystem.hpp"
#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <iostream>
#include <memory>
#include <string>

class Shell : public QObject {
  Q_OBJECT

public:
  Shell();
  Shell(const std::string &fsPath);
  void run();

private slots:
  void handleInput();

private:
  std::unique_ptr<VirtualFilesystem> vfs;
  std::unique_ptr<Parser> parser;
  QSocketNotifier *inputNotifier; // Изменено здесь
};

#endif