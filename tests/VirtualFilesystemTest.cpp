#include "commands/command.hpp"
#include "core/virtual_filesystem.hpp"
#include <algorithm>
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include <vector>

std::string sortLines(const std::string &input) {
  std::vector<std::string> lines;
  std::istringstream stream(input);
  std::string line;

  while (std::getline(stream, line)) {
    lines.push_back(line);
  }

  std::sort(lines.begin(), lines.end());
  std::ostringstream sortedStream;
  for (const auto &sortedLine : lines) {
    sortedStream << sortedLine << "\n";
  }

  return sortedStream.str();
}

class VirtualFilesystemTest : public ::testing::Test {
protected:
  std::shared_ptr<VirtualFilesystem> vfs;
  std::string archivePath = "fs.tar";

  VirtualFilesystemTest() {
    if (boost::filesystem::exists(archivePath)) {
      boost::filesystem::remove(archivePath);
    }
    vfs = std::make_shared<VirtualFilesystem>();
    vfs->addFileToStorage("/dir1", 0, FileType::DIR);
    vfs->addFileToStorage("/dir2", 0, FileType::DIR);
  }
};

// command: cd
TEST_F(VirtualFilesystemTest, TestChangeDirectorySuccess) {
  ChangeDirectoryCommand cdCommand(vfs);
  std::vector<std::string> args = {"/dir1"};
  EXPECT_EQ(cdCommand.execute(args), "");
  EXPECT_EQ(vfs->getCurrentDirectory(), "/dir1");
}

TEST_F(VirtualFilesystemTest, TestChangeDirectoryFailure) {
  ChangeDirectoryCommand cdCommand(vfs);
  std::vector<std::string> args = {"/nonexistent"};
  EXPECT_EQ(cdCommand.execute(args),
            "cd: no such file or directory: /nonexistent");
}

TEST_F(VirtualFilesystemTest, TestChangeDirectoryTooManyArguments) {
  ChangeDirectoryCommand cdCommand(vfs);
  std::vector<std::string> args = {"/dir1", "/dir2"};
  EXPECT_EQ(cdCommand.execute(args), "cd: too many arguments");
}

// command: ls
TEST_F(VirtualFilesystemTest, TestListDirectorySuccess) {
  ListDirectoryCommand lsCommand(vfs);
  std::vector<std::string> args = {"/"};
  std::string actualOutput = sortLines(lsCommand.execute(args));
  std::string expectedOutput = sortLines("dir2\ndir1\ndir\nhello");
  EXPECT_EQ(actualOutput, expectedOutput);
}

TEST_F(VirtualFilesystemTest, TestListDirectoryFailure) {
  ListDirectoryCommand lsCommand(vfs);
  std::vector<std::string> args = {"/nonexistent"};
  EXPECT_EQ(lsCommand.execute(args),
            "ls: cannot access '/nonexistent': Directory does not exist");
}

TEST_F(VirtualFilesystemTest, TestListDirectoryEmpty) {
  ListDirectoryCommand lsCommand(vfs);
  std::vector<std::string> args = {"/dir1"};
  EXPECT_EQ(lsCommand.execute(args), "ls: /dir1: No files found");
}

// command: cp
TEST_F(VirtualFilesystemTest, TestCopyFileSuccess) {
  CpCommand cpCommand(vfs);
  vfs->addFileToStorage("/file1", 100, FileType::REG);
  std::vector<std::string> args = {"/file1", "/dir1/file1"};
  EXPECT_EQ(cpCommand.execute(args), "");
}

TEST_F(VirtualFilesystemTest, TestCopyFileSourceNotFound) {
  CpCommand cpCommand(vfs);
  std::vector<std::string> args = {"/nonexistent", "/dir1/file1"};
  EXPECT_EQ(cpCommand.execute(args),
            "cp: source file or directory does not exist: /nonexistent");
}

TEST_F(VirtualFilesystemTest, TestCopyFileTargetExists) {
  CpCommand cpCommand(vfs);
  vfs->addFileToStorage("/file1", 100, FileType::REG);
  vfs->addFileToStorage("/dir1/file1", 100, FileType::REG);
  std::vector<std::string> args = {"/file1", "/dir1/file1"};
  EXPECT_EQ(cpCommand.execute(args), "cp: target already exists: /dir1/file1");
}

// command: tree
TEST_F(VirtualFilesystemTest, TestTreeCommandEmpty) {
  TreeCommand treeCommand(vfs);
  std::vector<std::string> args = {"/"};
  std::string actualOutput = sortLines(treeCommand.execute(args));
  std::string expectedOutput = sortLines("dir2\ndir1\ndir\n  file\n  dir2\nhello\n");
  EXPECT_EQ(actualOutput, expectedOutput);
}

TEST_F(VirtualFilesystemTest, TestTreeCommandDirectory) {
  TreeCommand treeCommand(vfs);
  vfs->addFileToStorage("/dir1/file1", 100, FileType::REG);
  std::vector<std::string> args = {"/dir1"};
  EXPECT_EQ(treeCommand.execute(args), "file1\n");
}

TEST_F(VirtualFilesystemTest, TestTreeCommandNoSuchDirectory) {
  TreeCommand treeCommand(vfs);
  std::vector<std::string> args = {"/nonexistent"};
  EXPECT_EQ(treeCommand.execute(args), "");
}

// command: find
TEST_F(VirtualFilesystemTest, TestFindCommandFileFound) {
  FindCommand findCommand(vfs);
  vfs->addFileToStorage("/dir1/file1", 100, FileType::REG);
  std::vector<std::string> args = {"file1"};
  EXPECT_EQ(findCommand.execute(args), "/dir1/file1\n");
}

TEST_F(VirtualFilesystemTest, TestFindCommandFileNotFound) {
  FindCommand findCommand(vfs);
  std::vector<std::string> args = {"nonexistent"};
  EXPECT_EQ(findCommand.execute(args), "");
}

TEST_F(VirtualFilesystemTest, TestFindCommandTooManyArguments) {
  FindCommand findCommand(vfs);
  std::vector<std::string> args = {"file1", "extra"};
  EXPECT_EQ(findCommand.execute(args), "find: missing argument");
}