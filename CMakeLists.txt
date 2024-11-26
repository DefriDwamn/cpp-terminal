cmake_minimum_required(VERSION 3.30)
project(cpp-terminal CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    nana
    GIT_REPOSITORY https://github.com/cnjinhao/nana.git
    GIT_TAG        v1.7.4
)
FetchContent_MakeAvailable(nana)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)
FetchContent_MakeAvailable(googletest)

find_package(Boost REQUIRED program_options filesystem system)
find_package(LibArchive REQUIRED)

include_directories("include" ${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.cpp" "main.cpp" "include/*.hpp")

add_executable(cpp-terminal ${SOURCES})

target_link_libraries(cpp-terminal PRIVATE nana Boost::program_options Boost::filesystem Boost::system LibArchive::LibArchive)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
add_subdirectory(tests)