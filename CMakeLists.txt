cmake_minimum_required(VERSION 3.30)
project(cpp-terminal CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LibArchive REQUIRED)
find_package(Boost REQUIRED program_options)
#find_package(GTest REQUIRED)

include_directories("include")

add_executable(${PROJECT_NAME}
    main.cpp
    src/core/shell.cpp
    src/core/virtual_filesystem.cpp
    src/core/file_storage.cpp
    src/core/parser.cpp
    src/commands/command.cpp
)

target_link_libraries(${PROJECT_NAME} LibArchive::LibArchive boost::boost)

#include_directories(include)