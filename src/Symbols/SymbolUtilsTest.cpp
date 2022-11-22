// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"
#include "Symbols/SymbolUtils.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_symbols {

using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using orbit_test_utils::HasValue;

TEST(GetStandardSymbolFilenamesForModule, ElfFile) {
  orbit_grpc_protos::ModuleInfo::ObjectFileType object_file_type =
      orbit_grpc_protos::ModuleInfo::kElfFile;
  std::filesystem::path directory = std::filesystem::path{"path"} / "to" / "folder";
  {  // .so file extension
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.so", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.so.debug"));
    EXPECT_THAT(file_names, testing::Contains("lib.so"));
  }

  {  // generic file extension (.ext)
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
  {  // .dll file extension
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.dll", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.dll.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
  }

  {  // generic file extension (.ext)
    const std::vector<std::filesystem::path> file_names =
        GetStandardSymbolFilenamesForModule(directory / "lib.ext", object_file_type);
    EXPECT_THAT(file_names, testing::Contains("lib.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext.pdb"));
    EXPECT_THAT(file_names, testing::Contains("lib.ext"));
  }
}

TEST(SymbolUtils, VerifySymbolFileByBuildId) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  {
    // ELF file with symbols and matching build id.
    const std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    const std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasNoError());
  }
  {
    // ELF file with symbols, but mis-matching build id.
    const std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    const std::string build_id = "incorrect build id";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("has a different build id"));
  }
  {
    // ELF file with matching build-id, but no symbols.
    const std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf";
    const std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("does not contain symbols"));
  }
  {
    // PDB file with symbols and matching build id.
    const std::filesystem::path symbols_file = testdata_directory / "dllmain.pdb";
    const std::string build_id = "92cdaeef73f74ebbbcf213b84f43b322-3";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasValue());
  }
  {
    // PDB file with symbols, but mis-matching build id.
    const std::filesystem::path symbols_file = testdata_directory / "dllmain.pdb";
    const std::string build_id = "incorrect build id";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("has a different build id"));
  }
  {
    // COFF file with matching build id, but no symbols.
    const std::filesystem::path symbols_file = testdata_directory / "dllmain.dll";
    const std::string build_id = "92cdaeef73f74ebbbcf213b84f43b322-3";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("does not contain symbols"));
  }
  {
    // File doesn't exist.
    const std::filesystem::path symbols_file = "path/to/invalid_file";
    const std::string build_id = "build id does not matter";
    const auto result = VerifySymbolFile(symbols_file, build_id);
    EXPECT_THAT(result, HasError("Unable to create symbols file"));
  }
}

TEST(SymbolUtils, VerifySymbolFileBySize) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  {
    // Symbol file of matching file size.
    std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    const auto file_size = orbit_base::FileSize(symbols_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifySymbolFile(symbols_file, file_size.value());
    EXPECT_THAT(result, HasNoError());
  }
  {
    // File of matching file size, but without symbols.
    std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf";
    const auto file_size = orbit_base::FileSize(symbols_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifySymbolFile(symbols_file, file_size.value());
    EXPECT_THAT(result, HasError("Unable to load symbols file"));
    EXPECT_THAT(result, HasError("File does not contain symbols"));
  }
  {
    // File with symbols, but of mis-matching file size.
    std::filesystem::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    const auto file_size = orbit_base::FileSize(symbols_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifySymbolFile(symbols_file, file_size.value() + 1);
    EXPECT_THAT(result, HasError("File size doesn't match"));
  }
}

TEST(SymbolUtils, VerifyObjectFile) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  {
    // ELF file with matching build id.
    const std::filesystem::path object_file = testdata_directory / "hello_world_elf";
    const std::string build_id = "d12d54bc5b72ccce54a408bdeda65e2530740ac8";
    const auto file_size = orbit_base::FileSize(object_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifyObjectFile(object_file, build_id, file_size.value());
    EXPECT_THAT(result, HasNoError());
  }
  {
    // ELF file with mis-matching build id.
    const std::filesystem::path object_file = testdata_directory / "hello_world_elf";
    const std::string build_id = "incorrect build-id";
    const auto file_size = orbit_base::FileSize(object_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifyObjectFile(object_file, build_id, file_size.value());
    EXPECT_THAT(result, HasError("has a different build id"));
  }
  {
    // ELF file with mis-matching size.
    const std::filesystem::path object_file = testdata_directory / "hello_world_elf";
    const std::string build_id = "d12d54bc5b72ccce54a408bdeda65e2530740ac8";
    const auto file_size = orbit_base::FileSize(object_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifyObjectFile(object_file, build_id, file_size.value() + 1);
    EXPECT_THAT(result, HasError("File size doesn't match"));
  }
  {
    // COFF file with matching build id.
    const std::filesystem::path object_file = testdata_directory / "dllmain.dll";
    const std::string build_id = "92cdaeef73f74ebbbcf213b84f43b322-3";
    const auto file_size = orbit_base::FileSize(object_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifyObjectFile(object_file, build_id, file_size.value());
    EXPECT_THAT(result, HasNoError());
  }
  {
    // COFF file with mis-matching build id.
    const std::filesystem::path object_file = testdata_directory / "dllmain.dll";
    const std::string build_id = "incorrect build-id";
    const auto file_size = orbit_base::FileSize(object_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifyObjectFile(object_file, build_id, file_size.value());
    EXPECT_THAT(result, HasError("has a different build id"));
  }
  {
    // COFF file with mis-matching size.
    const std::filesystem::path object_file = testdata_directory / "dllmain.dll";
    const std::string build_id = "92cdaeef73f74ebbbcf213b84f43b322-3";
    const auto file_size = orbit_base::FileSize(object_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifyObjectFile(object_file, build_id, file_size.value() + 1);
    EXPECT_THAT(result, HasError("File size doesn't match"));
  }
  {
    // PDB file.
    const std::filesystem::path object_file = testdata_directory / "dllmain.pdb";
    const std::string build_id = "92cdaeef73f74ebbbcf213b84f43b322-3";
    const auto file_size = orbit_base::FileSize(object_file);
    ASSERT_THAT(file_size, HasNoError());
    const auto result = VerifyObjectFile(object_file, build_id, file_size.value());
    EXPECT_THAT(result, HasError("The file was not recognized as a valid object file"));
  }
  {
    // File doesn't exist.
    const std::filesystem::path object_file = "path/to/nothing";
    const std::string build_id = "build id does not matter";
    const auto result = VerifyObjectFile(object_file, build_id, 42);
    EXPECT_THAT(result, HasError("Unable to load object file"));
  }
}

}  // namespace orbit_symbols