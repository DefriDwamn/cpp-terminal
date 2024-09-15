cmake_minimum_required(VERSION 3.30)
project(cpp-terminal CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(LibArchive REQUIRED)
find_package(Boost REQUIRED program_options)
#find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} LibArchive::LibArchive boost::boost)

#include_directories(include)