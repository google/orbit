// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>

#include "ObjectUtils/PdbFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/TestUtils.h"
#include "Test/Path.h"

using orbit_base::HasError;
using orbit_base::HasNoError;
using orbit_grpc_protos::SymbolInfo;
using ::testing::ElementsAre;

TEST(PdbFile, LoadDebugSymbols) {
  std::filesystem::path file_path_pdb = orbit_test::GetTestdataDir() / "dllmain.pdb";

  ErrorMessageOr<std::unique_ptr<orbit_object_utils::PdbFile>> pdb_file_result =
      orbit_object_utils::CreatePdbFile(file_path_pdb);
  ASSERT_THAT(pdb_file_result, HasNoError());
  std::unique_ptr<orbit_object_utils::PdbFile> pdb_file = std::move(pdb_file_result.value());
  auto symbols_result = pdb_file->LoadDebugSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  auto symbols = std::move(symbols_result.value());

  std::vector<SymbolInfo> symbol_infos(symbols.symbol_infos().begin(),
                                       symbols.symbol_infos().end());
  EXPECT_EQ(symbol_infos.size(), 4982);

  SymbolInfo symbol = symbol_infos[0];
  EXPECT_EQ(symbol.name(), "PrintHelloWorldInternal");
  EXPECT_EQ(symbol.demangled_name(), "PrintHelloWorldInternal");
  EXPECT_EQ(symbol.address(), 0xdc20);
  EXPECT_EQ(symbol.size(), 0x23);

  symbol = symbol_infos[1];
  EXPECT_EQ(symbol.name(), "PrintHelloWorld");
  EXPECT_EQ(symbol.demangled_name(), "PrintHelloWorld");
  EXPECT_EQ(symbol.address(), 0xdc50);
  EXPECT_EQ(symbol.size(), 0xa);
}

TEST(PdbFile, GetGuidAndAge) {
  std::filesystem::path file_path_pdb = orbit_test::GetTestdataDir() / "dllmain.pdb";

  ErrorMessageOr<std::unique_ptr<orbit_object_utils::PdbFile>> pdb_file_result =
      orbit_object_utils::CreatePdbFile(file_path_pdb);
  ASSERT_THAT(pdb_file_result, HasNoError());
  std::unique_ptr<orbit_object_utils::PdbFile> pdb_file = std::move(pdb_file_result.value());

  EXPECT_EQ(pdb_file->GetAge(), 1);

  std::array<uint8_t, 16> guid = pdb_file->GetGuid();
  EXPECT_THAT(std::vector(guid.begin(), guid.end()),
              ElementsAre(0xef, 0xae, 0xcd, 0x92, 0xf7, 0x73, 0xbb, 0x4e, 0xbc, 0xf2, 0x13, 0xb8,
                          0x4f, 0x43, 0xb3, 0x22));
}

TEST(PdbFile, CreatePdbFailsOnNonPdbFile) {
  // Any non-PDB file can be used here.
  std::filesystem::path file_path_pdb = orbit_test::GetTestdataDir() / "dllmain.dll";

  ErrorMessageOr<std::unique_ptr<orbit_object_utils::PdbFile>> pdb_file_result =
      orbit_object_utils::CreatePdbFile(file_path_pdb);
  EXPECT_THAT(pdb_file_result, HasError("Unable to load PDB file"));
}
