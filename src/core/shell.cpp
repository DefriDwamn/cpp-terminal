#include "core/shell.hpp"

Shell::Shell(const std::string &fsPath) {
    vfs = std::make_unique<VirtualFilesystem>(fsPath);
    parser = std::make_unique<Parser>(vfs.get());
}

Shell::Shell() {
    vfs = std::make_unique<VirtualFilesystem>("");
    parser = std::make_unique<Parser>(vfs.get());
}

void Shell::run() {
    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input == "exit")
            break;
        parser->processCommand(input);
    }
}
