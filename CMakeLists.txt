cmake_minimum_required(VERSION 3.30)
project(cpp-terminal CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Boost REQUIRED program_options)
find_package(QArchive)

include_directories("include" ${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp" "include/*.hpp")

add_executable(cpp-terminal ${SOURCES})


target_link_libraries(${PROJECT_NAME} boost::boost QArchive)
