#include "core/shell.hpp"

Shell::Shell() {
  vfs = std::make_unique<VirtualFilesystem>("");
  parser = std::make_unique<Parser>(vfs.get());

  inputNotifier =
      new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
  connect(inputNotifier, &QSocketNotifier::activated, this,
          &Shell::handleInput);
}

Shell::Shell(const std::string &fsPath) {
  vfs = std::make_unique<VirtualFilesystem>(fsPath);
  parser = std::make_unique<Parser>(vfs.get());

  inputNotifier =
      new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
  connect(inputNotifier, &QSocketNotifier::activated, this,
          &Shell::handleInput);
}

void Shell::handleInput() {
  std::string input;
  std::getline(std::cin, input);

  if (input == "exit") {
    QCoreApplication::quit();
  } else {
    parser->processCommand(input);
  }
}

void Shell::run() { std::cout << "Type 'exit' to quit." << std::endl; }
