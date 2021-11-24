// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <vector>

#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/CoffFile.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"
#include "absl/strings/ascii.h"

using orbit_grpc_protos::SymbolInfo;
using orbit_object_utils::CoffFile;
using orbit_object_utils::CreateCoffFile;
using orbit_object_utils::PdbDebugInfo;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

TEST(CoffFile, LoadDebugSymbols) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());
  std::unique_ptr<CoffFile> coff_file = std::move(coff_file_result.value());

  const auto symbols_result = coff_file->LoadDebugSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  EXPECT_EQ(symbols_result.value().symbols_file_path(), file_path);

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  EXPECT_EQ(symbol_infos.size(), 35);

  SymbolInfo& symbol_info = symbol_infos[4];
  EXPECT_EQ(symbol_info.name(), "pre_c_init");
  EXPECT_EQ(symbol_info.demangled_name(), "pre_c_init");
  uint64_t expected_address =
      0x0 + coff_file->GetExecutableSegmentOffset() + coff_file->GetLoadBias();
  EXPECT_EQ(symbol_info.address(), expected_address);
  EXPECT_EQ(symbol_info.size(), 0xc);

  symbol_info = symbol_infos[5];
  EXPECT_EQ(symbol_info.name(), "PrintHelloWorld");
  EXPECT_EQ(symbol_info.demangled_name(), "PrintHelloWorld");
  expected_address = 0x03a0 + coff_file->GetExecutableSegmentOffset() + coff_file->GetLoadBias();
  EXPECT_EQ(symbol_info.address(), expected_address);
  EXPECT_EQ(symbol_info.size(), 0x1b);
}

TEST(CoffFile, HasDebugSymbols) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());

  EXPECT_TRUE(coff_file_result.value()->HasDebugSymbols());
}

TEST(CoffFile, GetFilePath) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());

  EXPECT_EQ(coff_file_result.value()->GetFilePath(), file_path);
}

TEST(CoffFile, FileDoesNotExist) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "does_not_exist";

  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_TRUE(coff_file_or_error.has_error());
  EXPECT_THAT(absl::AsciiStrToLower(coff_file_or_error.error().message()),
              testing::HasSubstr("no such file or directory"));
}

TEST(CoffFile, LoadsPdbPathSuccessfully) {
  // Note that our test library libtest.dll does not have a PDB file path.
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "dllmain.dll";

  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_or_error, HasNoError());

  auto pdb_debug_info_or_error = coff_file_or_error.value()->GetDebugPdbInfo();
  ASSERT_THAT(pdb_debug_info_or_error, HasNoError());
  EXPECT_EQ("c:\\tmp\\dllmain.pdb", pdb_debug_info_or_error.value().pdb_file_path.string());

  // The correct loading of age and guid is tested in PdbFileTest, where we compare the
  // DLL and PDB data directly.
}

TEST(CoffFile, FailsWithErrorIfPdbDataNotPresent) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";
  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_or_error, HasNoError());

  auto pdb_debug_info_or_error = coff_file_or_error.value()->GetDebugPdbInfo();
  ASSERT_THAT(pdb_debug_info_or_error, HasError("Object file does not have debug PDB info."));
}

TEST(CoffFile, GetsCorrectBuildIdIfPdbInfoIsPresent) {
  // Note that our test library libtest.dll does not have a PDB file path.
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "dllmain.dll";

  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_or_error, HasNoError());

  EXPECT_EQ("efaecd92f773bb4ebcf213b84f43b322-3", coff_file_or_error.value()->GetBuildId());
}

TEST(CoffFile, GetsEmptyBuildIdIfPdbInfoIsNotPresent) {
  // Note that our test library libtest.dll does not have a PDB file path.
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_or_error, HasNoError());

  EXPECT_EQ("", coff_file_or_error.value()->GetBuildId());
}
