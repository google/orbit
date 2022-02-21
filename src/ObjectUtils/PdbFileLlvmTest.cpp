// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PdbFileLlvm.h"
#include "PdbFileTest.h"

using orbit_object_utils::PdbFileLlvm;

INSTANTIATE_TYPED_TEST_SUITE_P(PdbFileLlvmTest, PdbFileTest, ::testing::Types<PdbFileLlvm>);

// TODO(b/219413222): Once the parameter list gets also retrieved on the DIA implementation,
//  this test should be unified with the general PdbFileTest
TEST(PdbFileLlvmTest, LoadDebugSymbols) {
  std::filesystem::path file_path_pdb = orbit_test::GetTestdataDir() / "dllmain.pdb";

  ErrorMessageOr<std::unique_ptr<PdbFile>> pdb_file_result =
      PdbFileLlvm::CreatePdbFile(file_path_pdb, ObjectFileInfo{0x180000000, 0x1000});
  ASSERT_THAT(pdb_file_result, HasNoError());
  std::unique_ptr<orbit_object_utils::PdbFile> pdb_file = std::move(pdb_file_result.value());
  auto symbols_result = pdb_file->LoadDebugSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  auto symbols = std::move(symbols_result.value());

  absl::flat_hash_map<uint64_t, const SymbolInfo*> symbol_infos_by_address;
  for (const SymbolInfo& symbol_info : symbols.symbol_infos()) {
    symbol_infos_by_address.emplace(symbol_info.address(), &symbol_info);
  }

  {
    const SymbolInfo& symbol = *symbol_infos_by_address[0x18000ef90];
    EXPECT_EQ(symbol.demangled_name(), "PrintHelloWorldInternal()");
  }

  {
    const SymbolInfo& symbol = *symbol_infos_by_address[0x18000efd0];
    EXPECT_EQ(symbol.demangled_name(), "PrintHelloWorld()");
  }
}