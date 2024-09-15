#include <boost/program_options.hpp>
#include <iostream>

int main(int argc, char *argv[]) {
  try {
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}