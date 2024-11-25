#include "core/gui_shell.hpp"
#include "core/parser.hpp"
#include "core/virtual_filesystem.hpp"
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

std::string getFilesystemPath(const boost::program_options::variables_map &vm);

int main(int argc, char *argv[]) {
  namespace po = boost::program_options;

  try {
    po::options_description desc{"Options"};
    desc.add_options()("help,h", "Show help message")(
        "fs,f", po::value<std::string>(),
        "Path to the virtual filesystem in tar archive")(
        "create,c", "Create a new virtual filesystem");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      std::cout << desc;
      return 0;
    }

    po::notify(vm);

    if (vm.count("create")) {
      auto vfs = std::make_shared<VirtualFilesystem>();
      GUIShell shell(vfs);
      shell.run();
    } else if (vm.count("fs")) {
      std::string fsPath = getFilesystemPath(vm);
      auto vfs = std::make_shared<VirtualFilesystem>(fsPath);
      GUIShell shell(vfs);
      shell.run();
    } else {
      throw std::runtime_error(
          "Either --create or --fs option must be specified.");
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}

std::string getFilesystemPath(const boost::program_options::variables_map &vm) {
  std::string fsPath = vm["fs"].as<std::string>();
  std::ifstream file(fsPath);
  if (!file.good())
    throw std::runtime_error("File not found: " + fsPath);
  return fsPath;
}