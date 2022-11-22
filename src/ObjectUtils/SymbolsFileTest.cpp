// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "ObjectUtils/SymbolsFile.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_object_utils {

using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

TEST(SymbolsFile, CreateSymbolsFileFromElf) {
  const std::filesystem::path elf_with_symbols_path =
      orbit_test::GetTestdataDir() / "hello_world_elf";

  auto valid_symbols_file = CreateSymbolsFile(elf_with_symbols_path, ObjectFileInfo{0x10000});
  EXPECT_THAT(valid_symbols_file, HasNoError());

  const std::filesystem::path elf_without_symbols_path =
      orbit_test::GetTestdataDir() / "no_symbols_elf";

  auto invalid_symbols_file = CreateSymbolsFile(elf_without_symbols_path, ObjectFileInfo{0x10000});
  EXPECT_THAT(invalid_symbols_file, HasError("Unable to create symbols file"));
  EXPECT_THAT(invalid_symbols_file, HasError("File does not contain symbols."));
}

TEST(SymbolsFile, CreateSymbolsFileFromCoff) {
  const std::filesystem::path coff_with_symbols_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto valid_symbols_file = CreateSymbolsFile(coff_with_symbols_path, ObjectFileInfo{0x10000});
  EXPECT_THAT(valid_symbols_file, HasNoError());

  const std::filesystem::path coff_without_symbols_path =
      orbit_test::GetTestdataDir() / "dllmain.dll";

  auto invalid_symbols_file = CreateSymbolsFile(coff_without_symbols_path, ObjectFileInfo{0x10000});
  EXPECT_THAT(invalid_symbols_file, HasError("Unable to create symbols file"));
  EXPECT_THAT(invalid_symbols_file, HasError("File does not contain symbols."));
}

TEST(SymbolsFile, CreateSymbolsFileFromPdb) {
  const std::filesystem::path pwd_with_symbols_path = orbit_test::GetTestdataDir() / "dllmain.pdb";

  auto valid_symbols_file = CreateSymbolsFile(pwd_with_symbols_path, ObjectFileInfo{0x10000});
  EXPECT_THAT(valid_symbols_file, HasNoError());

  // pdb file always contains symbols, so a test for not containing symbols is not necessary
}

TEST(SymbolsFile, FailToCreateSymbolsFile) {
  const std::filesystem::path path_to_text_file = orbit_test::GetTestdataDir() / "textfile.txt";

  auto text_file = CreateSymbolsFile(path_to_text_file, ObjectFileInfo{0x10000});
  EXPECT_THAT(text_file, HasError("Unable to create symbols file"));
  EXPECT_THAT(text_file, HasError("File cannot be read as an object file"));
  EXPECT_THAT(text_file, HasError("File also cannot be read as a PDB file"));

  const std::filesystem::path invalid_path = orbit_test::GetTestdataDir() / "non_existing_file";
  auto invalid_file = CreateSymbolsFile(invalid_path, ObjectFileInfo{0x10000});
  EXPECT_THAT(invalid_file, HasError("Unable to create symbols file"));
  EXPECT_THAT(invalid_file, HasError("File does not exist"));
}

}  // namespace orbit_object_utils