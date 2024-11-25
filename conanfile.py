from conan import ConanFile

class cpp_terminal(ConanFile):
    name = "cpp-terminal"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = [
        "boost/1.86.0",
        "libarchive/3.7.6"
    ]
    default_options = {
        "boost/*:shared": False,
        "libarchive/*:shared": False,
    }
    generators = "CMakeDeps", "CMakeToolchain"
