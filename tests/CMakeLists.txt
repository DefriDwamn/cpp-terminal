project(tests)

add_executable(${PROJECT_NAME} VirtualFilesystemTest.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE gtest gtest_main LibArchive::LibArchive)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/commands/command.cpp" 
"${CMAKE_SOURCE_DIR}/src/core/virtual_filesystem.cpp"
"${CMAKE_SOURCE_DIR}/src/core/file_storage.cpp")

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

