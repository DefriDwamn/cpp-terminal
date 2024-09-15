#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

std::string getFilesystemPath(int argc, char *argv[]);

int main(int argc, char *argv[]) {
  try {
    std::cout << getFilesystemPath(argc, argv);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

std::string getFilesystemPath(int argc, char *argv[]) {
  namespace po = boost::program_options;

  po::options_description desc{"Options"};
  desc.add_options()("fs,f", po::value<std::string>()->required(),
                     "Path to the virtual filesystem in tar archive");

  po::variables_map vm;
  std::string fsPath;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("fs")) {
      fsPath = vm["fs"].as<std::string>();

      std::ifstream file(fsPath);
      if (!file.good()) {
        throw std::runtime_error("File not found: " + fsPath);
      }
    }
  } catch (const po::error &e) {
    throw std::runtime_error("Error parsing command line: " +
                             std::string(e.what()));
  } catch (const std::runtime_error &e) {
    throw;
  }

  return fsPath;
}