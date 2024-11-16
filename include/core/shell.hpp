#ifndef SHELL_HPP
#define SHELL_HPP
#include "core/parser.hpp"
#include "virtual_filesystem.hpp"
#include <QtCore/QCoreApplication>
#include <QtCore/QSocketNotifier>
#include <memory>
#include <qobject.h>
#include <qobjectdefs.h>
#include <qsocketnotifier.h>
#include <string>

class Shell : public QObject {
  Q_OBJECT

public:
  Shell();
  Shell(const std::string &fsPath);
  void run();

private:
  void handleInput();
  std::unique_ptr<VirtualFilesystem> vfs;
  std::unique_ptr<Parser> parser;
  bool running;
};

#endif