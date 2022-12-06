// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"
#include "Symbols/SymbolHelper.h"
#include "Test/Path.h"
#include "TestUtils/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_object_utils::ObjectFileInfo;
using orbit_symbols::SymbolHelper;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using orbit_test_utils::HasValue;

namespace fs = std::filesystem;

TEST(ReadSymbolsFile, Empty) {
  auto temp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::IsEmpty());
}

TEST(ReadSymbolsFile, EmptyWithComments) {
  auto temp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, HasNoError());

  ASSERT_THAT(
      orbit_base::WriteFully(temp_file_or_error.value().fd(),
                             "// C:\\Users\\username - Looks like a path but is a comment.\n"
                             "\t// A comment with a sneaky whitespace at the beginning.\n"
                             "\n"),  // Empty line as well
      HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::IsEmpty());
}

TEST(ReadSymbolsFile, OnePath) {
  auto temp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, HasNoError());

  ASSERT_THAT(orbit_base::WriteFully(temp_file_or_error.value().fd(),
                                     orbit_base::GetExecutableDir().string()),
              HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::ElementsAre(orbit_base::GetExecutableDir()));
}

TEST(ReadSymbolsFile, TwoPaths) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  auto temp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, HasNoError());

  ASSERT_THAT(orbit_base::WriteFully(temp_file_or_error.value().fd(),
                                     orbit_base::GetExecutableDir().string() + '\n'),
              HasNoError());
  ASSERT_THAT(
      orbit_base::WriteFully(temp_file_or_error.value().fd(), testdata_directory.string() + '\n'),
      HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::ElementsAre(orbit_base::GetExecutableDir(), testdata_directory));
}

TEST(ReadSymbolsFile, OnePathInQuotes) {
  auto temp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, HasNoError());

  ASSERT_THAT(
      orbit_base::WriteFully(temp_file_or_error.value().fd(),
                             absl::StrFormat("\"%s\"\n", orbit_base::GetExecutableDir().string())),
      HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::ElementsAre(orbit_base::GetExecutableDir()));
}

TEST(ReadSymbolsFile, OnePathTrailingWhitespace) {
  auto temp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(temp_file_or_error, HasNoError());

  ASSERT_THAT(
      orbit_base::WriteFully(temp_file_or_error.value().fd(),
                             absl::StrFormat("%s \t\n", orbit_base::GetExecutableDir().string())),
      HasNoError());

  std::vector<fs::path> paths =
      orbit_symbols::ReadSymbolsFile(temp_file_or_error.value().file_path());

  EXPECT_THAT(paths, testing::ElementsAre(orbit_base::GetExecutableDir()));
}

TEST(SymbolHelper, FindSymbolsFileLocally) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  const fs::path no_symbols_elf = testdata_directory / "no_symbols_elf";
  const fs::path no_symbols_elf_debug = testdata_directory / "no_symbols_elf.debug";
  const std::string no_symbols_elf_build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
  const fs::path dllmain_dll = testdata_directory / "dllmain.dll";
  const fs::path dllmain_pdb = testdata_directory / "dllmain.pdb";
  const std::string dllmain_build_id = "92cdaeef73f74ebbbcf213b84f43b322-3";

  SymbolHelper symbol_helper("", {});

  {  // Find .debug successfully - in directory
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        no_symbols_elf, no_symbols_elf_build_id, ModuleInfo::kElfFile, {testdata_directory});
    EXPECT_THAT(symbols_path_result, HasValue(no_symbols_elf_debug));
  }

  {  // Find .debug successfully - directly
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        no_symbols_elf, no_symbols_elf_build_id, ModuleInfo::kElfFile, {no_symbols_elf_debug});
    EXPECT_THAT(symbols_path_result, HasValue(no_symbols_elf_debug));
  }

  {  // Find .pdb successfully - in directory
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        dllmain_dll, dllmain_build_id, ModuleInfo::kCoffFile, {testdata_directory});
    EXPECT_THAT(symbols_path_result, HasValue(dllmain_pdb));
  }

  {  // Find .pdb successfully - directly
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        dllmain_dll, dllmain_build_id, ModuleInfo::kCoffFile, {dllmain_pdb});
    EXPECT_THAT(symbols_path_result, HasValue(dllmain_pdb));
  }

  {  // Non existing file (no matching file can be found for module filename)
    const fs::path non_existing_path = "file.not.exist";
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        non_existing_path, "irrelevant build id", ModuleInfo::kElfFile, {testdata_directory});
    EXPECT_THAT(symbols_path_result, HasError("Could not find"));
  }

  {  // Directly provided symbols file does not exist
    const fs::path symbols_path = testdata_directory / "file.not.exist";
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        "irrelevant module path", "irrelevant build id", ModuleInfo::kElfFile, {symbols_path});
    EXPECT_THAT(symbols_path_result, HasError("Could not find"));
  }

  {  // Find .debug fails because of wrong build id - in directory
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        no_symbols_elf, "wrong build id", ModuleInfo::kElfFile, {testdata_directory});
    EXPECT_THAT(symbols_path_result, HasError("Could not find"));
  }

  {  // Find .debug fails because of wrong build id - directly

    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        no_symbols_elf, "wrong build id", ModuleInfo::kElfFile, {no_symbols_elf_debug});
    EXPECT_THAT(symbols_path_result, HasError("Could not find"));
  }

  {  // Find .pdb fails because of wrong build id - in directory
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        dllmain_dll, "wrong build id", ModuleInfo::kCoffFile, {testdata_directory});
    EXPECT_THAT(symbols_path_result, HasError("Could not find"));
  }

  {  // Find .pdb fails because of wrong build id - directly
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        dllmain_dll, "wrong build id", ModuleInfo::kCoffFile, {dllmain_pdb});
    EXPECT_THAT(symbols_path_result, HasError("Could not find"));
  }

  {  // Find .debug fails because of module does not have build id
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        no_symbols_elf, "", ModuleInfo::kElfFile, {testdata_directory});
    EXPECT_THAT(symbols_path_result, HasError("Could not find"));
    EXPECT_THAT(symbols_path_result, HasError("does not contain a build id"));
  }

  {  // Find .pdb fails because of module does not have build id
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        dllmain_dll, "", ModuleInfo::kCoffFile, {testdata_directory});
    EXPECT_THAT(symbols_path_result, HasError("does not contain a build id"));
  }

  {  // Find .debug fails because of object_file_type is wrong
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        no_symbols_elf, no_symbols_elf_build_id, ModuleInfo::kCoffFile, {testdata_directory});
    EXPECT_THAT(
        symbols_path_result,
        HasError("Could not find a file with debug symbols on the local machine for module"));
  }

  {  // Find .pdb fails because of object_file_type is wrong
    const auto symbols_path_result = symbol_helper.FindSymbolsFileLocally(
        dllmain_dll, dllmain_build_id, ModuleInfo::kElfFile, {testdata_directory});
    EXPECT_THAT(
        symbols_path_result,
        HasError("Could not find a file with debug symbols on the local machine for module"));
  }
}

TEST(SymbolHelper, FindSymbolsInCacheBySize) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  SymbolHelper symbol_helper(testdata_directory, {});
  {
    // Same-size ELF file (smoke test).
    const fs::path file_name = "no_symbols_elf.debug";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = symbol_helper.FindSymbolsInCache(file_name, file_size.value());
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value(), file_path);
  }
  {
    // File in cache does not contain symbols.
    const fs::path file_name = "no_symbols_elf";
    const auto file_size = orbit_base::FileSize(testdata_directory / file_name);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = symbol_helper.FindSymbolsInCache(file_name, file_size.value());
    EXPECT_THAT(result, HasError("Unable to load symbols file"));
    EXPECT_THAT(result, HasError("File does not contain symbols"));
  }
  {
    // File in cache has different size.
    const fs::path file_name = "no_symbols_elf.debug";
    const auto file_size = orbit_base::FileSize(testdata_directory / file_name);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = symbol_helper.FindSymbolsInCache(file_name, file_size.value() + 1);
    EXPECT_THAT(result, HasError("File size doesn't match"));
  }
  {
    // File doesn't exist.
    const fs::path file_name = "non-existing_file";
    const auto result = symbol_helper.FindSymbolsInCache(file_name, 42);
    EXPECT_THAT(result, HasError("Unable to find symbols in cache"));
  }
}

TEST(SymbolHelper, FindSymbolsInCache) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  SymbolHelper symbol_helper(testdata_directory, {});
  {
    // Same ELF file (smoke test).
    const fs::path file_name = "no_symbols_elf.debug";
    const auto result =
        symbol_helper.FindSymbolsInCache(file_name, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value(), testdata_directory / file_name);
  }
  {
    // Same PDB file (smoke test).
    const fs::path file_name = "dllmain.pdb";
    const auto result =
        symbol_helper.FindSymbolsInCache(file_name, "92cdaeef73f74ebbbcf213b84f43b322-3");
    ASSERT_THAT(result, HasValue());
    EXPECT_THAT(result.value(), testdata_directory / file_name);
  }
  {
    // ELF file in cache does not have symbols.
    const fs::path file_name = "no_symbols_elf";
    const auto result =
        symbol_helper.FindSymbolsInCache(file_name, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    EXPECT_THAT(result, HasError("does not contain symbols"));
  }
  {
    // COFF file in cache does not have symbols.
    const fs::path file_name = "dllmain.dll";
    const auto result =
        symbol_helper.FindSymbolsInCache(file_name, "92cdaeef73f74ebbbcf213b84f43b322-3");
    EXPECT_THAT(result, HasError("does not contain symbols"));
  }
  {
    // ELF file in cache has different build id.
    const fs::path file_name = "no_symbols_elf.debug";
    const auto result = symbol_helper.FindSymbolsInCache(file_name, "non matching build id");
    EXPECT_THAT(result, HasError("has a different build id"));
  }
  {
    // PDB in cache has different build id.
    const fs::path file_name = "dllmain.pdb";
    const auto result = symbol_helper.FindSymbolsInCache(file_name, "non matching build id");
    EXPECT_THAT(result, HasError("has a different build id"));
  }
  {
    // File doesn't exist.
    const fs::path file_name = "non-existing_file";
    const auto result = symbol_helper.FindSymbolsInCache(file_name, "unimportant build id");
    EXPECT_THAT(result, HasError("Unable to find symbols in cache"));
  }
}

TEST(SymbolHelper, FindObjectInCache) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  SymbolHelper symbol_helper(testdata_directory, {});
  {
    // ELF file.
    const fs::path file_name = "hello_world_elf";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = symbol_helper.FindObjectInCache(
        file_name, "d12d54bc5b72ccce54a408bdeda65e2530740ac8", file_size.value());
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value(), file_path);
  }
  {
    // ELF file has a different build id.
    const fs::path file_name = "hello_world_elf";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasNoError());
    const auto result =
        symbol_helper.FindObjectInCache(file_name, "non-matching build id", file_size.value());
    ASSERT_THAT(result, HasError("has a different build id"));
  }
  {
    // ELF file has a different size.
    const fs::path file_name = "hello_world_elf";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = symbol_helper.FindObjectInCache(
        file_name, "d12d54bc5b72ccce54a408bdeda65e2530740ac8", file_size.value() + 1);
    ASSERT_THAT(result, HasError("File size doesn't match"));
  }
  {
    // COFF file.
    const fs::path file_name = "dllmain.dll";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = symbol_helper.FindObjectInCache(
        file_name, "92cdaeef73f74ebbbcf213b84f43b322-3", file_size.value());
    ASSERT_THAT(result, HasValue());
    EXPECT_EQ(result.value(), file_path);
  }
  {
    // COFF file has a different build id.
    const fs::path file_name = "dllmain.dll";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasNoError());
    const auto result =
        symbol_helper.FindObjectInCache(file_name, "non-matching build id", file_size.value());
    ASSERT_THAT(result, HasError("has a different build id"));
  }
  {
    // COFF file has a different size.
    const fs::path file_name = "dllmain.dll";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = symbol_helper.FindObjectInCache(
        file_name, "92cdaeef73f74ebbbcf213b84f43b322-3", file_size.value() + 1);
    ASSERT_THAT(result, HasError("File size doesn't match"));
  }
  {
    // PDB file.
    const fs::path file_name = "dllmain.pdb";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = symbol_helper.FindObjectInCache(
        file_name, "92cdaeef73f74ebbbcf213b84f43b322-3", file_size.value());
    EXPECT_THAT(result, HasError("The file was not recognized as a valid object file"));
  }
  {
    // File doesn't exist.
    const fs::path file_name = "non-existing_file";
    const fs::path file_path = testdata_directory / file_name;
    const auto file_size = orbit_base::FileSize(file_path);
    ASSERT_THAT(file_size, HasError(""));
    const auto result = symbol_helper.FindObjectInCache(file_name, "unimportant build id", 42);
    EXPECT_THAT(result, HasError("Unable to find object file in cache"));
  }
}

TEST(SymbolHelper, LoadSymbolsFromFile) {
  std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  {
    // .debug ELF file contains symbols.
    const fs::path file_path = testdata_directory / "no_symbols_elf.debug";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path, ObjectFileInfo{0x10000});

    ASSERT_THAT(result, HasValue());
    const ModuleSymbols& symbols = result.value();

    EXPECT_FALSE(symbols.symbol_infos().empty());
  }
  {
    // .pdb contains symbols.
    const fs::path file_path = testdata_directory / "dllmain.pdb";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path, ObjectFileInfo{0x10000});

    ASSERT_THAT(result, HasValue());
    const ModuleSymbols& symbols = result.value();

    EXPECT_FALSE(symbols.symbol_infos().empty());
  }
  {
    // ELF file does not contain symbols.
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path, ObjectFileInfo{0x10000});
    EXPECT_THAT(result, HasError("does not contain symbols"));
  }
  {
    // COFF file does not contain symbols.
    const fs::path file_path = testdata_directory / "dllmain.dll";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path, ObjectFileInfo{0x10000});
    EXPECT_THAT(result, HasError("does not contain symbols"));
  }
  {
    // File doesn't exist.
    const fs::path file_path = testdata_directory / "file_does_not_exist";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path, ObjectFileInfo{0x10000});
    EXPECT_THAT(result, HasError("Unable to create symbols file"));
    EXPECT_THAT(result, HasError("File does not exist"));
  }
}

TEST(SymbolHelper, LoadFallbackSymbolsFromFile) {
  std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  {
    // ELF file with symbols.
    const fs::path file_path = testdata_directory / "hello_world_elf";
    const auto module_symbols_or_error = SymbolHelper::LoadFallbackSymbolsFromFile(file_path);
    EXPECT_THAT(module_symbols_or_error, HasValue());
    EXPECT_FALSE(module_symbols_or_error.value().symbol_infos().empty());
  }
  {
    // ELF file without symbols.
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const auto module_symbols_or_error = SymbolHelper::LoadFallbackSymbolsFromFile(file_path);
    EXPECT_THAT(module_symbols_or_error, HasValue());
    EXPECT_FALSE(module_symbols_or_error.value().symbol_infos().empty());
  }
  {
    // COFF file without symbols.
    const fs::path file_path = testdata_directory / "dllmain.dll";
    const auto module_symbols_or_error = SymbolHelper::LoadFallbackSymbolsFromFile(file_path);
    EXPECT_THAT(module_symbols_or_error, HasValue());
    EXPECT_FALSE(module_symbols_or_error.value().symbol_infos().empty());
  }
  {
    // PDB file.
    const fs::path file_path = testdata_directory / "dllmain.pdb";
    const auto module_symbols_or_error = SymbolHelper::LoadFallbackSymbolsFromFile(file_path);
    EXPECT_THAT(module_symbols_or_error, HasError("Unable to load object file"));
  }
  {
    // Files doesn't exist.
    const fs::path file_path = testdata_directory / "file_does_not_exist";
    const auto result = SymbolHelper::LoadFallbackSymbolsFromFile(file_path);
    EXPECT_THAT(result, HasError("Unable to load object file"));
    EXPECT_THAT(result, HasError("such file or directory"));
  }
}

TEST(SymbolHelper, GenerateCachedFilePath) {
  fs::path fake_cache_dir = "/path/to/cache";
  SymbolHelper symbol_helper{fake_cache_dir, {}};
  const std::filesystem::path file_path = "/var/data/filename.elf";
  const std::filesystem::path cache_file_path = fake_cache_dir / "_var_data_filename.elf";
  EXPECT_EQ(symbol_helper.GenerateCachedFilePath(file_path), cache_file_path);
}

TEST(SymbolHelper, FindDebugInfoFileLocally) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  SymbolHelper symbol_helper("", {});
  constexpr uint32_t kExpectedChecksum = 0x2bf887bf;

  const auto symbols_path_result = symbol_helper.FindDebugInfoFileLocally(
      "hello_world_elf.debug", kExpectedChecksum, {testdata_directory});
  ASSERT_THAT(symbols_path_result, HasValue());
  EXPECT_EQ(symbols_path_result.value().filename(), "hello_world_elf.debug");
  EXPECT_EQ(symbols_path_result.value().parent_path(), testdata_directory);
}

TEST(SymbolHelper, IsMatchingDebugInfoFile) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  constexpr uint32_t kExpectedChecksum = 0x2bf887bf;
  auto correct_file_path = testdata_directory / "hello_world_elf.debug";
  auto existing_but_wrong_file_path = testdata_directory / "hello_world_elf";
  auto non_existing_file_path = testdata_directory / "hello_world_elf.does_not_exist";
  EXPECT_TRUE(SymbolHelper::IsMatchingDebugInfoFile(correct_file_path, kExpectedChecksum));
  EXPECT_FALSE(
      SymbolHelper::IsMatchingDebugInfoFile(existing_but_wrong_file_path, kExpectedChecksum));
  EXPECT_FALSE(SymbolHelper::IsMatchingDebugInfoFile(non_existing_file_path, kExpectedChecksum));
}

TEST(SymbolHelper, FindSymbolsInStructedDebugStore) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  SymbolHelper symbol_helper("", {testdata_directory / "debugstore"});

  const fs::path file_path = testdata_directory / "no_symbols_elf";
  const fs::path symbols_path = testdata_directory / "debugstore" / ".build-id" / "b5" /
                                "413574bbacec6eacb3b89b1012d0e2cd92ec6b.debug";

  const auto symbols_path_result =
      symbol_helper.FindSymbolsFileLocally(file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b",
                                           ModuleInfo::kElfFile, {testdata_directory});

  ASSERT_THAT(symbols_path_result, HasValue());
  EXPECT_EQ(symbols_path_result.value(), symbols_path);
}

TEST(FileStartsWithDeprecationNote, FileDoesNotExist) {
  ErrorMessageOr<bool> error_result =
      orbit_symbols::FileStartsWithDeprecationNote("non/existing/path/");

  EXPECT_THAT(error_result, HasError("Unable to open file"));
}

TEST(FileStartsWithDeprecationNote, EmptyFile) {
  auto tmp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, HasNoError());

  ErrorMessageOr<bool> result =
      orbit_symbols::FileStartsWithDeprecationNote(tmp_file_or_error.value().file_path());

  ASSERT_THAT(result, HasValue());
  EXPECT_FALSE(result.value());
}

TEST(FileStartsWithDeprecationNote, NoDeprecationNote) {
  auto tmp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, HasNoError());

  ASSERT_THAT(orbit_base::WriteFully(tmp_file_or_error.value().fd(),
                                     "Some file content.\nC:\\path\n\\\\ This is a comment"),
              HasNoError());

  ErrorMessageOr<bool> result =
      orbit_symbols::FileStartsWithDeprecationNote(tmp_file_or_error.value().file_path());

  ASSERT_THAT(result, HasValue());
  EXPECT_FALSE(result.value());
}

TEST(FileStartsWithDeprecationNote, HasDeprecationNote) {
  auto tmp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, HasNoError());

  ASSERT_THAT(orbit_base::WriteFully(
                  tmp_file_or_error.value().fd(),
                  "// !!! Do not remove this comment !!!\n// This file has been migrated in Orbit "
                  "1.68. Please use: Menu > Settings > Symbol Locations...\n// This file can still "
                  "used by Orbit versions prior to 1.68. If that is relevant to you, do not delete "
                  "this file.\n"),
              HasNoError());
  ASSERT_THAT(
      orbit_base::WriteFully(tmp_file_or_error.value().fd(), "Some more content.\n// Comment"),
      HasNoError());

  ErrorMessageOr<bool> result =
      orbit_symbols::FileStartsWithDeprecationNote(tmp_file_or_error.value().file_path());

  ASSERT_THAT(result, HasValue());
  EXPECT_TRUE(result.value());
}

TEST(AddDeprecationNoteToFile, FileDoesNotExist) {
  ErrorMessageOr<void> error_result = orbit_symbols::AddDeprecationNoteToFile("non/existing/path/");

  EXPECT_THAT(error_result, HasError("Unable to open file"));
}

TEST(AddDeprecationNoteToFile, AddNote) {
  auto tmp_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(tmp_file_or_error, HasNoError());
  orbit_test_utils::TemporaryFile& file{tmp_file_or_error.value()};

  constexpr std::string_view kFileContent = "Some file content.\nC:\\path\n\\\\ This is a comment";
  ASSERT_THAT(orbit_base::WriteFully(file.fd(), kFileContent), HasNoError());

  {
    ErrorMessageOr<void> add_result = orbit_symbols::AddDeprecationNoteToFile(file.file_path());
    ASSERT_THAT(add_result, HasValue());

    ErrorMessageOr<bool> check_result =
        orbit_symbols::FileStartsWithDeprecationNote(file.file_path());
    ASSERT_THAT(check_result, HasValue());
    EXPECT_TRUE(check_result.value());
  }

  {  // Adding the deprecation note, when there already is one, will fail
    ErrorMessageOr<void> add_result = orbit_symbols::AddDeprecationNoteToFile(file.file_path());
    EXPECT_FALSE(add_result.has_value());

    ErrorMessageOr<bool> check_result =
        orbit_symbols::FileStartsWithDeprecationNote(file.file_path());
    ASSERT_THAT(check_result, HasValue());
    EXPECT_TRUE(check_result.value());
  }
}