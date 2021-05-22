// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"
#include "OrbitBase/TemporaryFile.h"
#include "OrbitBase/TestUtils.h"

namespace orbit_base {

using ::testing::HasSubstr;

TEST(File, DefaultUniqueFdIsInvalidDescriptor) {
  unique_fd fd;
  EXPECT_FALSE(fd.valid());
}

TEST(File, EmptyUnuqueFdCanBeReleased) {
  unique_fd fd;
  fd.release();
  EXPECT_FALSE(fd.valid());
}

TEST(File, MoveAssingToExisingUniqueFd) {
  unique_fd fd;

  auto fd_or_error =
      OpenFileForReading(GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");

  ASSERT_TRUE(fd_or_error.has_value()) << fd_or_error.error().message();
  fd = std::move(fd_or_error.value());
  EXPECT_TRUE(fd.valid());
  EXPECT_FALSE(fd_or_error.value().valid());
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
TEST(File, UniqueFdSelfMove) {
  auto fd_or_error =
      OpenFileForReading(GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");
  ASSERT_TRUE(fd_or_error.has_value()) << fd_or_error.error().message();
  unique_fd valid_fd{std::move(fd_or_error.value())};

  valid_fd = std::move(valid_fd);

  ASSERT_TRUE(valid_fd.valid());
}
#pragma GCC diagnostic pop

TEST(File, AcccessInvalidUniqueFd) {
  unique_fd fd;
  EXPECT_FALSE(fd.valid());
  EXPECT_DEATH((void)fd.get(), "");

  auto fd_or_error =
      OpenFileForReading(GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");

  fd = std::move(fd_or_error.value());
  EXPECT_TRUE(fd.valid());
  fd.release();
  EXPECT_FALSE(fd.valid());
  EXPECT_DEATH((void)fd.get(), "");
}

TEST(File, OpenFileForReadingInvalidFile) {
  const auto fd_or_error = OpenFileForReading("non/existing/filename");
  ASSERT_TRUE(fd_or_error.has_error());
}

TEST(File, OpenNewFileForReadWrite) {
  auto temporary_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  auto fd_or_error = OpenNewFileForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(fd_or_error.has_error());
  EXPECT_THAT(fd_or_error.error().message(), HasSubstr("File exists"));
  temporary_file.CloseAndRemove();
  fd_or_error = OpenNewFileForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(fd_or_error.has_value()) << fd_or_error.error().message();
}

TEST(File, OpenExistingFileForReadWrite) {
  auto temporary_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  auto fd_or_error = OpenExistingFileForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(fd_or_error.has_value()) << fd_or_error.error().message();
  fd_or_error.value().release();
  temporary_file.CloseAndRemove();
  fd_or_error = OpenExistingFileForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(fd_or_error.has_error());
  EXPECT_THAT(fd_or_error.error().message(), HasSubstr("No such file or directory"));
}

TEST(File, WriteFullySmoke) {
  auto temporary_file_or_error = TemporaryFile::Create();
  CHECK(temporary_file_or_error.has_value());
  TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  // Write buffer into file.
  const std::string buffer = "blub\nbla\n";
  ErrorMessageOr<void> write_result_or_error = WriteFully(temporary_file.fd(), buffer);
  ASSERT_FALSE(write_result_or_error.has_error()) << write_result_or_error.error().message();

  // Read back and compare content.
  ErrorMessageOr<unique_fd> fd_or_error = OpenFileForReading(temporary_file.file_path().string());
  ASSERT_FALSE(fd_or_error.has_error()) << fd_or_error.error().message();
  std::array<char, 64> read_back = {};
  ErrorMessageOr<size_t> result_or_error =
      ReadFully(fd_or_error.value(), read_back.data(), buffer.size() + 3);
  ASSERT_FALSE(result_or_error.has_error()) << result_or_error.error().message();
  EXPECT_EQ(result_or_error.value(), buffer.size());
  EXPECT_STREQ(read_back.data(), buffer.c_str());
}

TEST(File, WriteFullyAtOffsetSmoke) {
  auto temporary_file_or_error = TemporaryFile::Create();
  CHECK(temporary_file_or_error.has_value());
  TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  // Write at the beginning of the previously empty file.
  std::array<char, 64> buffer{'a', 'b', '\n', 'c', 'd', '\n'};
  ErrorMessageOr<void> write_result_or_error =
      WriteFullyAtOffset(temporary_file.fd(), buffer.data(), 6, 0);
  ASSERT_FALSE(write_result_or_error.has_error()) << write_result_or_error.error().message();

  // Read back and compare content.
  std::array<char, 64> read_back = {};
  ErrorMessageOr<unique_fd> fd_or_error = OpenFileForReading(temporary_file.file_path().string());
  ASSERT_FALSE(fd_or_error.has_error()) << fd_or_error.error().message();
  ErrorMessageOr<size_t> result_or_error = ReadFully(fd_or_error.value(), read_back.data(), 64);
  ASSERT_FALSE(result_or_error.has_error()) << result_or_error.error().message();
  EXPECT_EQ(result_or_error.value(), 6);
  EXPECT_STREQ(read_back.data(), buffer.data());

  // Overrride at a given offset.
  buffer = {'e', 'f'};
  write_result_or_error = WriteFullyAtOffset(temporary_file.fd(), buffer.data(), 2, 3);
  ASSERT_FALSE(write_result_or_error.has_error()) << write_result_or_error.error().message();

  // Read back and compare content.
  read_back = {};
  fd_or_error = OpenFileForReading(temporary_file.file_path().string());
  ASSERT_FALSE(fd_or_error.has_error()) << fd_or_error.error().message();
  result_or_error = ReadFully(fd_or_error.value(), read_back.data(), 64);
  ASSERT_FALSE(result_or_error.has_error()) << result_or_error.error().message();
  EXPECT_EQ(result_or_error.value(), 6);
  EXPECT_STREQ(read_back.data(), "ab\nef\n");
}

TEST(File, ReadFullySmoke) {
  const auto fd_or_error =
      OpenFileForReading(GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");
  ASSERT_FALSE(fd_or_error.has_error()) << fd_or_error.error().message();
  const auto& fd = fd_or_error.value();
  ASSERT_TRUE(fd.valid());
  std::array<char, 64> buf{};

  ErrorMessageOr<size_t> result = ReadFully(fd, buf.data(), 5);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 5);
  EXPECT_STREQ(buf.data(), "conte");

  buf = {};

  result = ReadFully(fd, buf.data(), buf.size());
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 11);
  EXPECT_STREQ(buf.data(), "nt\nnew line");

  buf = {};

  result = ReadFully(fd, buf.data(), buf.size());
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 0);
  EXPECT_STREQ(buf.data(), "");
}

TEST(File, ReadFullyAtOffsetSmoke) {
  const auto fd_or_error =
      OpenFileForReading(GetExecutableDir() / "testdata" / "OrbitBase" / "textfile.bin");
  CHECK(!fd_or_error.has_error());
  const auto& fd = fd_or_error.value();
  CHECK(fd.valid());
  std::array<char, 64> buf{};

  // Read at beginning of file.
  ErrorMessageOr<size_t> result = ReadFullyAtOffset(fd, buf.data(), 5, 0);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 5);
  EXPECT_STREQ(buf.data(), "conte");

  buf = {};

  // Read entire file (even past the end of the file).
  result = ReadFullyAtOffset(fd, buf.data(), 64, 0);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 16);
  EXPECT_STREQ(buf.data(), "content\nnew line");

  buf = {};

  // Read entire rest of the file (even past the end of the file) starting from an offset.
  result = ReadFullyAtOffset(fd, buf.data(), 64, 5);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 11);
  EXPECT_STREQ(buf.data(), "nt\nnew line");

  buf = {};

  // Read something in the middle of the file.
  result = ReadFullyAtOffset(fd, buf.data(), 2, 7);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 2);
  EXPECT_STREQ(buf.data(), "\nn");

  buf = {};

  // Read something after the end of the file.
  result = ReadFullyAtOffset(fd, buf.data(), 1, 16);
  ASSERT_FALSE(result.has_error()) << result.error().message();
  EXPECT_EQ(result.value(), 0);
  EXPECT_STREQ(buf.data(), "");
}

TEST(File, ReadStructureAtOffset) {
  struct TestStructure {
    uint64_t u64;
    std::array<char, 8> char_array;
  };

  auto tmp_file_or_error = TemporaryFile::Create();
  ASSERT_TRUE(tmp_file_or_error.has_value()) << tmp_file_or_error.error().message();

  auto fd_or_error = OpenExistingFileForReadWrite(tmp_file_or_error.value().file_path());
  ASSERT_TRUE(fd_or_error.has_value()) << fd_or_error.error().message();

  auto fd = std::move(fd_or_error.value());

  constexpr const off_t kOffset = 42;
  constexpr uint64_t kU64Value = 1121;
  constexpr const char* kCharArray = "abcdefg";

  auto write_result = WriteFullyAtOffset(fd, &kU64Value, sizeof(kU64Value), kOffset);
  ASSERT_FALSE(write_result.has_error()) << write_result.error().message();

  // Make sure we cannot read the structure - the file is too small
  auto test_struct_or_error = ReadFullyAtOffset<TestStructure>(fd, kOffset);
  ASSERT_TRUE(test_struct_or_error.has_error());
  ASSERT_EQ(test_struct_or_error.error().message(), "Not enough bytes left in the file: 8 < 16");

  write_result = WriteFullyAtOffset(fd, kCharArray, 8, kOffset + sizeof(kU64Value));
  ASSERT_FALSE(write_result.has_error()) << write_result.error().message();

  // Now we should be able to read the structure
  test_struct_or_error = ReadFullyAtOffset<TestStructure>(fd, kOffset);
  ASSERT_TRUE(test_struct_or_error.has_value()) << test_struct_or_error.error().message();

  const auto test_struct = test_struct_or_error.value();

  EXPECT_EQ(test_struct.u64, kU64Value);
  EXPECT_STREQ(test_struct.char_array.data(), kCharArray);
}

TEST(File, MoveFile) {
  auto tmp_file_or_error = TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, HasNoError());
  auto tmp_file = std::move(tmp_file_or_error.value());
  std::filesystem::path new_path = tmp_file.file_path().string() + ".new";

  auto exists_or_error = FileExists(new_path);
  ASSERT_THAT(exists_or_error, HasNoError());
  ASSERT_FALSE(exists_or_error.value());

  exists_or_error = FileExists(tmp_file.file_path());
  ASSERT_THAT(exists_or_error, HasNoError());
  EXPECT_TRUE(exists_or_error.value());

  auto move_result = MoveFile(tmp_file.file_path(), new_path);
  ASSERT_THAT(move_result, HasNoError());

  exists_or_error = FileExists(new_path);
  ASSERT_THAT(exists_or_error, HasNoError());
  EXPECT_TRUE(exists_or_error.value());

  exists_or_error = FileExists(tmp_file.file_path());
  ASSERT_THAT(exists_or_error, HasNoError());
  EXPECT_FALSE(exists_or_error.value());

  remove(new_path.string().c_str());
}

}  // namespace orbit_base