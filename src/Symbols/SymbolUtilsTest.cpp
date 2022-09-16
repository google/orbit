// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "Symbols/SymbolUtils.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_symbols {

using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

TEST(GetStandardSymbolFilenamesForModule, ElfFile) {
  orbit_grpc_protos::ModuleInfo::ObjectFileType object_file_type =
      orbit_grpc_protos::ModuleInfo::kElfFile;
  std::filesystem::path directory = std::filesystem::path{"path"} / "to" / "folder";
  {  // .so file extention
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.so", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.so.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.so"));
  }

  {  // generic file extention (.ext)
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.ext", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext"));
  }
}

TEST(GetStandardSymbolFilenamesForModule, CoffFile) {
  orbit_grpc_protos::ModuleInfo::ObjectFileType object_file_type =
      orbit_grpc_protos::ModuleInfo::kCoffFile;
  std::filesystem::path directory = std::filesystem::path{"C:"} / "path" / "to" / "folder";
  {  // .dll file extention
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.dll", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.dll.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
  }

  {  // generic file extention (.ext)
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.ext", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext"));
  }
}

TEST(VerifySymbolFile, BuildId) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  {
    // valid elf file containing symbols and matching build id
    std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasValue());
  }
  {
    // valid elf file containing symbols, build id not matching;
    std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    std::string build_id = "incorrect build id";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("has a different build id"));
  }
  {
    // valid elf file no symbols and matching build id
    std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf";
    std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("does not contain symbols"));
  }
  {
    // valid pdb file containing symbols and matching build id
    std::filesystem::path symbols_file = testdata_directory / "dllmain.pdb";
    std::string build_id = "efaecd92f773bb4ebcf213b84f43b322-3";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasValue());
  }
  {
    // valid pdb file containing symbols, build id not matching
    std::filesystem::path symbols_file = testdata_directory / "dllmain.pdb";
    std::string build_id = "incorrect build id";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("has a different build id"));
  }
  {
    // valid coff file no symbols and matching build id
    std::filesystem::path symbols_file = testdata_directory / "dllmain.dll";
    std::string build_id = "efaecd92f773bb4ebcf213b84f43b322-3";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("does not contain symbols"));
  }
  {
    // invalid file
    std::filesystem::path symbols_file = "path/to/invalid_file";
    std::string build_id = "build id does not matter";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("Unable to create symbols file"));
  }
}

TEST(VerifySymbolFile, FileSize) {
  constexpr uint64_t kNoSymbolsElfDebugFileSize = 45856;
  constexpr uint64_t kIncorrectSymbolsFileSize = 123456;

  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  {
    // a file of matching file size
    std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    const auto result = VerifySymbolFile(symbols_file, kNoSymbolsElfDebugFileSize);
    EXPECT_THAT(result, HasValue());
  }
  {
    // a file of mis-matching file size
    std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    const auto result = VerifySymbolFile(symbols_file, kIncorrectSymbolsFileSize);
    EXPECT_THAT(result, HasError("file size doesn't match"));
  }
}

}  // namespace orbit_symbols