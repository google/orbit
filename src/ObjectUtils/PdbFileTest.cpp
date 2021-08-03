// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>

#include "ObjectUtils/CoffFile.h"
#include "ObjectUtils/PdbFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/TestUtils.h"
#include "Test/Path.h"

using orbit_base::HasError;
using orbit_base::HasNoError;
using orbit_grpc_protos::SymbolInfo;
using orbit_object_utils::CreateCoffFile;
using orbit_object_utils::PdbDebugInfo;
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
  EXPECT_EQ(symbol_infos.size(), 5469);

  SymbolInfo symbol = symbol_infos[0];
  EXPECT_EQ(symbol.name(), "PrintHelloWorldInternal");
  EXPECT_EQ(symbol.demangled_name(), "PrintHelloWorldInternal");
  EXPECT_EQ(symbol.address(), 0xdf90);
  EXPECT_EQ(symbol.size(), 0x2b);

  symbol = symbol_infos[1];
  EXPECT_EQ(symbol.name(), "PrintHelloWorld");
  EXPECT_EQ(symbol.demangled_name(), "PrintHelloWorld");
  EXPECT_EQ(symbol.address(), 0xdfd0);
  EXPECT_EQ(symbol.size(), 0xe);
}

TEST(PdbFile, CanObtainGuidAndAgeFromPdbAndDll) {
  std::filesystem::path file_path_pdb = orbit_test::GetTestdataDir() / "dllmain.pdb";

  ErrorMessageOr<std::unique_ptr<orbit_object_utils::PdbFile>> pdb_file_result =
      orbit_object_utils::CreatePdbFile(file_path_pdb);
  ASSERT_THAT(pdb_file_result, HasNoError());
  std::unique_ptr<orbit_object_utils::PdbFile> pdb_file = std::move(pdb_file_result.value());

  // We load the PDB debug info from the dll to see if it matches the data in the pdb.
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "dllmain.dll";

  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_or_error, HasNoError());

  auto pdb_debug_info_or_error = coff_file_or_error.value()->GetDebugPdbInfo();
  ASSERT_THAT(pdb_debug_info_or_error, HasNoError());

  EXPECT_EQ(pdb_file->GetAge(), pdb_debug_info_or_error.value().age);
  EXPECT_THAT(pdb_file->GetGuid(), testing::ElementsAreArray(pdb_debug_info_or_error.value().guid));
}

TEST(PdbFile, CreatePdbFailsOnNonPdbFile) {
  // Any non-PDB file can be used here.
  std::filesystem::path file_path_pdb = orbit_test::GetTestdataDir() / "dllmain.dll";

  ErrorMessageOr<std::unique_ptr<orbit_object_utils::PdbFile>> pdb_file_result =
      orbit_object_utils::CreatePdbFile(file_path_pdb);
  EXPECT_THAT(pdb_file_result, HasError("Unable to load PDB file"));
}
