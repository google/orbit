// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PdbFileTest.h"

// Must be included after PdbFileTest.h
#include "PdbFileDia.h"

using orbit_object_utils::PdbFileDia;

INSTANTIATE_TYPED_TEST_SUITE_P(PdbFileDiaTest, PdbFileTest, ::testing::Types<PdbFileDia>);

// This test is specific to using the DIA SDK to load PDB files.
TEST(PdbFileDiaTest, CreatePdbDoesNotFailOnCoInitializeWhenAlreadyInitialized) {
  std::filesystem::path file_path_pdb = orbit_test::GetTestdataDir() / "dllmain.pdb";
  HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  ASSERT_TRUE(result == S_OK || result == S_FALSE);
  ErrorMessageOr<std::unique_ptr<PdbFile>> pdb_file_result1 =
      PdbFileDia::CreatePdbFile(file_path_pdb, ObjectFileInfo{0x180000000, 0x1000});
  ASSERT_THAT(pdb_file_result1, HasNoError());
  CoUninitialize();
}

// This test is specific to using the DIA SDK to load PDB files.
TEST(PdbFileDiaTest, PdbFileProperlyUninitializesComLibrary) {
  std::filesystem::path file_path_pdb = orbit_test::GetTestdataDir() / "dllmain.pdb";
  {
    ErrorMessageOr<std::unique_ptr<PdbFile>> pdb_file_result =
        PdbFileDia::CreatePdbFile(file_path_pdb, ObjectFileInfo{0x180000000, 0x1000});
    ASSERT_THAT(pdb_file_result, HasNoError());
  }

  HRESULT result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  // This would be S_FALSE if the PdbFileDia class didn't properly balance its CoInitialize() call.
  ASSERT_EQ(result, S_OK);
  CoUninitialize();
}
