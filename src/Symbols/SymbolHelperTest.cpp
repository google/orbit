// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/ascii.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/TemporaryFile.h"
#include "OrbitPaths/Paths.h"
#include "Symbols/SymbolHelper.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"
#include "symbol.pb.h"

using orbit_grpc_protos::ModuleSymbols;
using orbit_symbols::SymbolHelper;
using orbit_test_utils::HasError;
namespace fs = std::filesystem;

static const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();

TEST(ReadSymbolsFile, Empty) {
  auto temp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, orbit_test_utils::HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::IsEmpty());
}

TEST(ReadSymbolsFile, EmptyWithComments) {
  auto temp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, orbit_test_utils::HasNoError());

  ASSERT_THAT(
      orbit_base::WriteFully(temp_file_or_error.value().fd(),
                             "// C:\\Users\\username - Looks like a path but is a comment.\n"
                             "\t// A comment with a sneaky whitespace at the beginning.\n"
                             "\n"),  // Empty line as well
      orbit_test_utils::HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::IsEmpty());
}

TEST(ReadSymbolsFile, OnePath) {
  auto temp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, orbit_test_utils::HasNoError());

  ASSERT_THAT(orbit_base::WriteFully(temp_file_or_error.value().fd(),
                                     orbit_base::GetExecutableDir().string()),
              orbit_test_utils::HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::ElementsAre(orbit_base::GetExecutableDir()));
}

TEST(ReadSymbolsFile, TwoPaths) {
  auto temp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, orbit_test_utils::HasNoError());

  ASSERT_THAT(orbit_base::WriteFully(temp_file_or_error.value().fd(),
                                     orbit_base::GetExecutableDir().string() + '\n'),
              orbit_test_utils::HasNoError());
  ASSERT_THAT(
      orbit_base::WriteFully(temp_file_or_error.value().fd(), testdata_directory.string() + '\n'),
      orbit_test_utils::HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::ElementsAre(orbit_base::GetExecutableDir(), testdata_directory));
}

TEST(ReadSymbolsFile, OnePathInQuotes) {
  auto temp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, orbit_test_utils::HasNoError());

  ASSERT_THAT(
      orbit_base::WriteFully(temp_file_or_error.value().fd(),
                             absl::StrFormat("\"%s\"\n", orbit_base::GetExecutableDir().string())),
      orbit_test_utils::HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::ElementsAre(orbit_base::GetExecutableDir()));
}

TEST(ReadSymbolsFile, OnePathTrailingWhitespace) {
  auto temp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, orbit_test_utils::HasNoError());

  ASSERT_THAT(
      orbit_base::WriteFully(temp_file_or_error.value().fd(),
                             absl::StrFormat("%s \t\n", orbit_base::GetExecutableDir().string())),
      orbit_test_utils::HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::ElementsAre(orbit_base::GetExecutableDir()));
}

TEST(SymbolHelper, FindSymbolsFileLocally) {
  SymbolHelper symbol_helper("", {});
  {
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const fs::path symbols_path = testdata_directory / "no_symbols_elf.debug";

    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b", {testdata_directory});
    ASSERT_FALSE(symbols_path_result.has_error()) << symbols_path_result.error().message();
    EXPECT_EQ(symbols_path_result.value(), symbols_path);
  }

  {
    const fs::path non_existing_path = "file.not.exist";
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        non_existing_path, "irrelevant build id", {testdata_directory});
    ASSERT_TRUE(symbols_path_result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("could not find"));
  }

  {
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const auto symbols_path_result =
        symbol_helper.FindSymbolsFileLocally(file_path, "wrong build id", {testdata_directory});
    ASSERT_TRUE(symbols_path_result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("could not find"));
  }

  {
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const auto symbols_path_result =
        symbol_helper.FindSymbolsFileLocally(file_path, "", {testdata_directory});
    ASSERT_TRUE(symbols_path_result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("could not find"));
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("does not contain a build id"));
  }
}

TEST(SymbolHelper, FindSymbolsInCache) {
  SymbolHelper symbol_helper(testdata_directory, {});

  // This is more of a smoke test (looking for the same file)
  {
    const fs::path file = "no_symbols_elf.debug";
    const auto result =
        symbol_helper.FindSymbolsInCache(file, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    ASSERT_FALSE(result.has_error()) << result.error().message();
    EXPECT_EQ(result.value(), testdata_directory / file);
  }

  // file in cache does not have symbols
  {
    const fs::path file_path = "no_symbols_elf";
    const auto result =
        symbol_helper.FindSymbolsInCache(file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("does not contain symbols"));
  }

  // file in cache has different build id
  {
    const fs::path file_path = "no_symbols_elf.debug";
    const auto result = symbol_helper.FindSymbolsInCache(file_path, "non matching build id");
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("has a different build id"));
  }
}

TEST(SymbolHelper, LoadFromFile) {
  // contains symbols
  {
    const fs::path file_path = testdata_directory / "no_symbols_elf.debug";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_FALSE(result.has_error()) << result.error().message();
    const ModuleSymbols& symbols = result.value();

    EXPECT_EQ(symbols.symbols_file_path(), file_path);
    EXPECT_FALSE(symbols.symbol_infos().empty());
  }

  // does not contain symbols
  {
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("does not have a .symtab section"));
  }

  // invalid file
  {
    const fs::path file_path = testdata_directory / "file_does_not_exist";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no such file or directory"));
  }
}

TEST(SymbolHelper, GenerateCachedFileName) {
  SymbolHelper symbol_helper{orbit_paths::CreateOrGetCacheDir(), {}};
  const std::filesystem::path file_path = "/var/data/filename.elf";
  const std::filesystem::path cache_file_path =
      orbit_paths::CreateOrGetCacheDir() / "_var_data_filename.elf";
  EXPECT_EQ(symbol_helper.GenerateCachedFileName(file_path), cache_file_path);
}

TEST(SymbolHelper, VerifySymbolsFile) {
  {
    // valid file containing symbols and matching build id
    fs::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const auto result = SymbolHelper::VerifySymbolsFile(symbols_file, build_id);
    EXPECT_FALSE(result.has_error()) << result.error().message();
  }
  {
    // valid file containing symbols, build id not matching;
    fs::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    std::string build_id = "incorrect build id";
    const auto result = SymbolHelper::VerifySymbolsFile(symbols_file, build_id);
    EXPECT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("has a different build id"));
  }
  {
    // valid file no symbols and matching build id
    fs::path symbols_file = testdata_directory / "no_symbols_elf";
    std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const auto result = SymbolHelper::VerifySymbolsFile(symbols_file, build_id);
    EXPECT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("does not contain symbols"));
  }
  {
    // invalid file
    fs::path symbols_file = "path/to/invalid_file";
    std::string build_id = "build id does not matter";
    const auto result = SymbolHelper::VerifySymbolsFile(symbols_file, build_id);
    EXPECT_TRUE(result.has_error());
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("unable to load object file"));
  }
}

TEST(SymbolHelper, FindDebugInfoFileLocally) {
  SymbolHelper symbol_helper("", {});
  constexpr uint32_t kExpectedChecksum = 0x2bf887bf;

  const auto symbols_path_result = symbol_helper.FindDebugInfoFileLocally(
      "hello_world_elf.debug", kExpectedChecksum, {testdata_directory});
  ASSERT_FALSE(symbols_path_result.has_error()) << symbols_path_result.error().message();
  EXPECT_EQ(symbols_path_result.value().filename(), "hello_world_elf.debug");
  EXPECT_EQ(symbols_path_result.value().parent_path(), testdata_directory);
}

TEST(SymbolHelper, IsMatchingDebugInfoFile) {
  constexpr uint32_t kExpectedChecksum = 0x2bf887bf;
  auto correct_file_path = testdata_directory / "hello_world_elf.debug";
  auto existing_but_wrong_file_path = testdata_directory / "hello_world_elf";
  auto non_existing_file_path = testdata_directory / "hello_world_elf.does_not_exist";
  EXPECT_TRUE(SymbolHelper::IsMatchingDebugInfoFile(correct_file_path, kExpectedChecksum));
  EXPECT_FALSE(
      SymbolHelper::IsMatchingDebugInfoFile(existing_but_wrong_file_path, kExpectedChecksum));
  EXPECT_FALSE(SymbolHelper::IsMatchingDebugInfoFile(non_existing_file_path, kExpectedChecksum));
}

TEST(SymbolHelper, FindDebugInfoFileInDebugStore) {
  const fs::path symbols_path = testdata_directory / "debugstore" / ".build-id" / "b5" /
                                "413574bbacec6eacb3b89b1012d0e2cd92ec6b.debug";
  const std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";

  ErrorMessageOr<std::filesystem::path> error_or_path =
      SymbolHelper::FindDebugInfoFileInDebugStore(testdata_directory / "debugstore", build_id);

  ASSERT_TRUE(error_or_path.has_value()) << error_or_path.error().message();
  EXPECT_EQ(error_or_path.value(), symbols_path);
}

TEST(SymbolHelper, FindSymbolsInStructedDebugStore) {
  SymbolHelper symbol_helper("", {testdata_directory / "debugstore"});

  const fs::path file_path = testdata_directory / "no_symbols_elf";
  const fs::path symbols_path = testdata_directory / "debugstore" / ".build-id" / "b5" /
                                "413574bbacec6eacb3b89b1012d0e2cd92ec6b.debug";

  const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
      file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b", {testdata_directory});

  ASSERT_FALSE(symbols_path_result.has_error()) << symbols_path_result.error().message();
  EXPECT_EQ(symbols_path_result.value(), symbols_path);
}

TEST(FileStartsWithDeprecationNote, FileDoesNotExist) {
  ErrorMessageOr<bool> error_result =
      orbit_symbols::FileStartsWithDeprecationNote("non/existing/path/");

  EXPECT_THAT(error_result, orbit_test_utils::HasError("Unable to open file"));
}

TEST(FileStartsWithDeprecationNote, EmptyFile) {
  auto tmp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, orbit_test_utils::HasNoError());

  ErrorMessageOr<bool> result =
      orbit_symbols::FileStartsWithDeprecationNote(tmp_file_or_error.value().file_path());

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result.value());
}

TEST(FileStartsWithDeprecationNote, NoDeprecationNote) {
  auto tmp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, orbit_test_utils::HasNoError());

  ASSERT_THAT(orbit_base::WriteFully(tmp_file_or_error.value().fd(),
                                     "Some file content.\nC:\\path\n\\\\ This is a comment"),
              orbit_test_utils::HasNoError());

  ErrorMessageOr<bool> result =
      orbit_symbols::FileStartsWithDeprecationNote(tmp_file_or_error.value().file_path());

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result.value());
}

TEST(FileStartsWithDeprecationNote, HasDeprecationNote) {
  auto tmp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, orbit_test_utils::HasNoError());

  ASSERT_THAT(orbit_base::WriteFully(
                  tmp_file_or_error.value().fd(),
                  "// !!! Do not remove this comment !!!\n// This file has been migrated in Orbit "
                  "1.68. Please use: Menu > Settings > Symbol Locations...\n// This file can still "
                  "used by Orbit versions prior to 1.68. If that is relevant to you, do not delete "
                  "this file.\n"),
              orbit_test_utils::HasNoError());
  ASSERT_THAT(
      orbit_base::WriteFully(tmp_file_or_error.value().fd(), "Some more content.\n// Comment"),
      orbit_test_utils::HasNoError());

  ErrorMessageOr<bool> result =
      orbit_symbols::FileStartsWithDeprecationNote(tmp_file_or_error.value().file_path());

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result.value());
}

TEST(AddDeprecationNoteToFile, FileDoesNotExist) {
  ErrorMessageOr<void> error_result = orbit_symbols::AddDeprecationNoteToFile("non/existing/path/");

  EXPECT_THAT(error_result, orbit_test_utils::HasError("Unable to open file"));
}

TEST(AddDeprecationNoteToFile, AddNote) {
  auto tmp_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, orbit_test_utils::HasNoError());
  orbit_base::TemporaryFile& file{tmp_file_or_error.value()};

  constexpr std::string_view kFileContent = "Some file content.\nC:\\path\n\\\\ This is a comment";
  ASSERT_THAT(orbit_base::WriteFully(file.fd(), kFileContent), orbit_test_utils::HasNoError());

  {
    ErrorMessageOr<void> add_result = orbit_symbols::AddDeprecationNoteToFile(file.file_path());
    ASSERT_TRUE(add_result.has_value());

    ErrorMessageOr<bool> check_result =
        orbit_symbols::FileStartsWithDeprecationNote(file.file_path());
    ASSERT_TRUE(check_result.has_value());
    EXPECT_TRUE(check_result.value());
  }

  {  // Adding the deprecation note, when there already is one, will fail
    ErrorMessageOr<void> add_result = orbit_symbols::AddDeprecationNoteToFile(file.file_path());
    EXPECT_FALSE(add_result.has_value());

    ErrorMessageOr<bool> check_result =
        orbit_symbols::FileStartsWithDeprecationNote(file.file_path());
    ASSERT_TRUE(check_result.has_value());
    EXPECT_TRUE(check_result.value());
  }
}