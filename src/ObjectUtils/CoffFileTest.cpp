// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/ascii.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <google/protobuf/stubs/port.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/CoffFile.h"
#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/Result.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

using orbit_grpc_protos::SymbolInfo;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

namespace orbit_object_utils {

TEST(CoffFile, LoadDebugSymbols) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());
  std::unique_ptr<CoffFile> coff_file = std::move(coff_file_result.value());

  EXPECT_TRUE(coff_file->HasDebugSymbols());

  const auto symbols_result = coff_file->LoadDebugSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  EXPECT_EQ(symbol_infos.size(), 53);

  // Size from the corresponding RUNTIME_FUNCTION.
  SymbolInfo& symbol_info = symbol_infos[0];
  EXPECT_EQ(symbol_info.demangled_name(), "pre_c_init");
  uint64_t expected_address =
      0x0 + coff_file->GetExecutableSegmentOffset() + coff_file->GetLoadBias();
  EXPECT_EQ(symbol_info.address(), expected_address);
  EXPECT_EQ(symbol_info.size(), 0xc);
  EXPECT_EQ(symbol_info.is_hotpatchable(), false);

  symbol_info = symbol_infos[7];
  EXPECT_EQ(symbol_info.demangled_name(), "PrintHelloWorld");
  expected_address = 0x03a0 + coff_file->GetExecutableSegmentOffset() + coff_file->GetLoadBias();
  EXPECT_EQ(symbol_info.address(), expected_address);
  EXPECT_EQ(symbol_info.size(), 0x1b);
  EXPECT_EQ(symbol_info.is_hotpatchable(), false);

  symbol_info = symbol_infos.back();
  EXPECT_EQ(symbol_info.demangled_name(), "register_frame_ctor");
  expected_address = 0x1300 + coff_file->GetExecutableSegmentOffset() + coff_file->GetLoadBias();
  EXPECT_EQ(symbol_info.address(), expected_address);
  EXPECT_EQ(symbol_info.size(), 0x5);
  EXPECT_EQ(symbol_info.is_hotpatchable(), false);

  // Size deduced as the distance from this function's address and the next function's address.
  symbol_info = symbol_infos[34];
  EXPECT_EQ(symbol_info.demangled_name(), "vfprintf");
  expected_address = 0x1090 + coff_file->GetExecutableSegmentOffset() + coff_file->GetLoadBias();
  EXPECT_EQ(symbol_info.address(), expected_address);
  EXPECT_EQ(symbol_info.size(), 0x8);  // One six-byte jump plus two bytes of padding.
  EXPECT_EQ(symbol_info.is_hotpatchable(), false);
}

TEST(CoffFile, HasDebugSymbols) {
  std::filesystem::path coff_file_with_symbols_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_with_symbols_result = CreateCoffFile(coff_file_with_symbols_path);
  ASSERT_THAT(coff_file_with_symbols_result, HasNoError());

  EXPECT_TRUE(coff_file_with_symbols_result.value()->HasDebugSymbols());

  std::filesystem::path coff_file_without_symbols_path =
      orbit_test::GetTestdataDir() / "dllmain.dll";

  auto coff_file_without_symbols_result = CreateCoffFile(coff_file_without_symbols_path);
  ASSERT_THAT(coff_file_without_symbols_result, HasNoError());

  EXPECT_FALSE(coff_file_without_symbols_result.value()->HasDebugSymbols());
}

static ::testing::Matcher<SymbolInfo> SymbolInfoEq(std::string_view demangled_name,
                                                   uint64_t address, uint64_t size,
                                                   bool is_hotpatchable) {
  return testing::AllOf(
      testing::Property("demangled_name", &SymbolInfo::demangled_name, demangled_name),
      testing::Property("address", &SymbolInfo::address, address),
      testing::Property("size", &SymbolInfo::size, size),
      testing::Property("is_hotpatchable", &SymbolInfo::is_hotpatchable, is_hotpatchable));
}

TEST(CoffFile, LoadSymbolsFromExportTable) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());
  const std::unique_ptr<CoffFile>& coff_file = coff_file_result.value();

  EXPECT_TRUE(coff_file->HasExportTable());

  const auto symbols_result = coff_file->LoadSymbolsFromExportTable();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  EXPECT_THAT(symbol_infos, testing::ElementsAre(SymbolInfoEq(
                                "PrintHelloWorld", /*address=*/coff_file->GetLoadBias() + 0x13a0,
                                /*size=*/27, /*is_hotpatchable=*/false)));
}

TEST(CoffFile, LoadSymbolsFromExportTableOneExportedOnlyByOrdinal) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "exports_one_by_ordinal.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());
  const std::unique_ptr<CoffFile>& coff_file = coff_file_result.value();

  EXPECT_TRUE(coff_file->HasExportTable());

  const auto symbols_result = coff_file->LoadSymbolsFromExportTable();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  const uint64_t image_base = coff_file->GetLoadBias();
  EXPECT_THAT(
      symbol_infos,
      testing::ElementsAre(SymbolInfoEq("NONAME1", /*address=*/image_base + 0x1110, /*size=*/43,
                                        /*is_hotpatchable=*/false),
                           SymbolInfoEq("PrintHelloWorldNamed", /*address=*/image_base + 0x1150,
                                        /*size=*/43, /*is_hotpatchable=*/false)));
}

TEST(CoffFile, LoadSymbolsFromExportTableAllExportedOnlyByOrdinal) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "exports_all_by_ordinal.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());
  const std::unique_ptr<CoffFile>& coff_file = coff_file_result.value();

  EXPECT_TRUE(coff_file->HasExportTable());

  const auto symbols_result = coff_file->LoadSymbolsFromExportTable();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  const uint64_t image_base = coff_file->GetLoadBias();
  EXPECT_THAT(symbol_infos,
              testing::ElementsAre(SymbolInfoEq("NONAME1", /*address=*/image_base + 0x1110,
                                                /*size=*/43, /*is_hotpatchable=*/false),
                                   SymbolInfoEq("NONAME2", /*address=*/image_base + 0x1150,
                                                /*size=*/43, /*is_hotpatchable=*/false)));
}

TEST(CoffFile, LoadSymbolsFromExportTableNoExportTable) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "no_export_table.exe";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());
  const std::unique_ptr<CoffFile>& coff_file = coff_file_result.value();

  EXPECT_FALSE(coff_file->HasExportTable());

  const auto symbols_result = coff_file->LoadSymbolsFromExportTable();
  ASSERT_THAT(symbols_result, HasError("PE/COFF file does not have an Export Table"));
}

TEST(CoffFile, LoadExceptionTableEntriesAsSymbolsNoChainedInfo) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());
  const std::unique_ptr<CoffFile>& coff_file = coff_file_result.value();

  const auto symbols_result = coff_file->LoadExceptionTableEntriesAsSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  EXPECT_EQ(symbol_infos.size(), 38);
  for (const SymbolInfo& symbol_info : symbol_infos) {
    EXPECT_EQ(symbol_info.demangled_name(),
              absl::StrFormat("[function@%#x]", symbol_info.address()));
  }
  // Verify a couple of functions.
  // Ground truth can be deduced from `dumpbin libtest.dll /UNWINDINFO`.
  // The corresponding function can then be obtained from
  // `dumpbin libtest.dll /SYMBOLS | findstr /c:"notype ()"`.
  EXPECT_THAT(symbol_infos[0], SymbolInfoEq("[function@0x62641000]", /*address=*/0x62641000,
                                            /*size=*/12, /*is_hotpatchable=*/false));  // pre_c_init
  EXPECT_THAT(symbol_infos[3],
              SymbolInfoEq("[function@0x62641350]", /*address=*/0x62641350, /*size=*/18,
                           /*is_hotpatchable=*/false));  // DllMainCRTStartup
  EXPECT_THAT(symbol_infos[7],
              SymbolInfoEq("[function@0x626413a0]", /*address=*/0x626413a0, /*size=*/27,
                           /*is_hotpatchable=*/false));  // PrintHelloWorld
}

TEST(CoffFile, LoadExceptionTableEntriesAsSymbolsWithChainedInfo) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "dllmain.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());
  const std::unique_ptr<CoffFile>& coff_file = coff_file_result.value();

  const auto symbols_result = coff_file->LoadExceptionTableEntriesAsSymbols();
  ASSERT_THAT(symbols_result, HasNoError());

  std::vector<SymbolInfo> symbol_infos(symbols_result.value().symbol_infos().begin(),
                                       symbols_result.value().symbol_infos().end());
  // Verify all the functions for which there is chained unwind info.
  // Ground truth can be deduced from `dumpbin dllmain.dll /UNWINDINFO` looking for "CHAININFO".
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq(
                                "[function@0x180086400]", /*address=*/0x180086400,
                                /*size=*/0x1800864b5 - 0x180086400, /*is_hotpatchable=*/false)));
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq(
                                "[function@0x180090500]", /*address=*/0x180090500,
                                /*size=*/0x180090929 - 0x180090500, /*is_hotpatchable=*/false)));
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq(
                                "[function@0x180090b50]", /*address=*/0x180090b50,
                                /*size=*/0x180090ef8 - 0x180090b50, /*is_hotpatchable=*/false)));
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq(
                                "[function@0x180090ff0]", /*address=*/0x180090ff0,
                                /*size=*/0x1800910dd - 0x180090ff0, /*is_hotpatchable=*/false)));
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq(
                                "[function@0x180091c70]", /*address=*/0x180091c70,
                                /*size=*/0x180091deb - 0x180091c70, /*is_hotpatchable=*/false)));
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq(
                                "[function@0x180092510]", /*address=*/0x180092510,
                                /*size=*/0x1800928e0 - 0x180092510, /*is_hotpatchable=*/false)));
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq(
                                "[function@0x1800c2220]", /*address=*/0x1800c2220,
                                /*size=*/0x1800c22dc - 0x1800c2220, /*is_hotpatchable=*/false)));
  EXPECT_THAT(symbol_infos, testing::Contains(SymbolInfoEq(
                                "[function@0x1800c2350]", /*address=*/0x1800c2350,
                                /*size=*/0x1800c26ed - 0x1800c2350, /*is_hotpatchable=*/false)));
}

TEST(CoffFile, LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbols) {
  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file = CreateObjectFile(file_path);
  ASSERT_THAT(coff_file, HasNoError());
  ASSERT_TRUE(coff_file.value()->IsCoff());

  const auto fallback_symbols =
      coff_file.value()->LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbols();
  ASSERT_THAT(fallback_symbols, HasNoError());

  std::vector<SymbolInfo> symbol_infos(fallback_symbols.value().symbol_infos().begin(),
                                       fallback_symbols.value().symbol_infos().end());
  ASSERT_EQ(symbol_infos.size(), 38);
  EXPECT_THAT(symbol_infos.front(),
              SymbolInfoEq("[function@0x62641000]", /*address=*/0x62641000, /*size=*/12,
                           /*is_hotpatchable=*/false));  // `pre_c_init`
  EXPECT_THAT(symbol_infos.at(6),
              SymbolInfoEq("[function@0x62641390]", /*address=*/0x62641390, /*size=*/1,
                           /*is_hotpatchable=*/false));  // `__gcc_deregister_frame`
  EXPECT_THAT(symbol_infos.at(7), SymbolInfoEq("PrintHelloWorld", /*address=*/0x626413a0,
                                               /*size=*/27, /*is_hotpatchable=*/false));
  EXPECT_THAT(symbol_infos.at(8),
              SymbolInfoEq("[function@0x626413c0]", /*address=*/0x626413c0, /*size=*/58,
                           /*is_hotpatchable=*/false));  // `__do_global_dtors`
  EXPECT_THAT(symbol_infos.back(),
              SymbolInfoEq("[function@0x62642300]", /*address=*/0x62642300, /*size=*/5,
                           /*is_hotpatchable=*/false));  // `register_frame_ctor`
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
  EXPECT_EQ("C:\\tmp\\dllmain.pdb", pdb_debug_info_or_error.value().pdb_file_path.string());

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

  EXPECT_EQ("afd69a4f7f394e5088fc34477bd0bae3-1", coff_file_or_error.value()->GetBuildId());
}

TEST(CoffFile, GetsEmptyBuildIdIfPdbInfoIsNotPresent) {
  // Note that our test library libtest.dll does not have a PDB file path.
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";

  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_or_error, HasNoError());

  EXPECT_EQ("", coff_file_or_error.value()->GetBuildId());
}

TEST(CoffFile, GetLoadBiasAndExecutableSegmentOffsetAndImageSize) {
  {
    std::filesystem::path file_path = orbit_test::GetTestdataDir() / "dllmain.dll";
    auto coff_file_or_error = CreateCoffFile(file_path);
    ASSERT_THAT(coff_file_or_error, HasNoError());
    const CoffFile& coff_file = *coff_file_or_error.value();
    EXPECT_EQ(coff_file.GetLoadBias(), 0x180000000);
    EXPECT_EQ(coff_file.GetExecutableSegmentOffset(), 0x1000);
    EXPECT_EQ(coff_file.GetImageSize(), 0x10d000);
  }
  {
    std::filesystem::path file_path = orbit_test::GetTestdataDir() / "libtest.dll";
    auto coff_file_or_error = CreateCoffFile(file_path);
    ASSERT_THAT(coff_file_or_error, HasNoError());
    const CoffFile& coff_file = *coff_file_or_error.value();
    EXPECT_EQ(coff_file.GetLoadBias(), 0x62640000);
    EXPECT_EQ(coff_file.GetExecutableSegmentOffset(), 0x1000);
    EXPECT_EQ(coff_file.GetImageSize(), 0x20000);
  }
}

TEST(CoffFile, ObjectSegments) {
  std::filesystem::path file_path = orbit_test::GetTestdataDir() / "dllmain.dll";
  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_or_error, HasNoError());
  const std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment>& segments =
      coff_file_or_error.value()->GetObjectSegments();
  ASSERT_EQ(segments.size(), 8);

  EXPECT_EQ(segments[0].offset_in_file(), 0x400);
  EXPECT_EQ(segments[0].size_in_file(), 0xCEA00);
  EXPECT_EQ(segments[0].address(), 0x180001000);
  EXPECT_EQ(segments[0].size_in_memory(), 0xCE9E4);

  EXPECT_EQ(segments[1].offset_in_file(), 0xCEE00);
  EXPECT_EQ(segments[1].size_in_file(), 0x27A00);
  EXPECT_EQ(segments[1].address(), 0x1800D0000);
  EXPECT_EQ(segments[1].size_in_memory(), 0x2797D);

  EXPECT_EQ(segments[2].offset_in_file(), 0xF6800);
  EXPECT_EQ(segments[2].size_in_file(), 0x2800);
  EXPECT_EQ(segments[2].address(), 0x1800F8000);
  EXPECT_EQ(segments[2].size_in_memory(), 0x5269);

  EXPECT_EQ(segments[3].offset_in_file(), 0xF9000);
  EXPECT_EQ(segments[3].size_in_file(), 0x9000);
  EXPECT_EQ(segments[3].address(), 0x1800FE000);
  EXPECT_EQ(segments[3].size_in_memory(), 0x8F4C);

  EXPECT_EQ(segments[4].offset_in_file(), 0x102000);
  EXPECT_EQ(segments[4].size_in_file(), 0x1200);
  EXPECT_EQ(segments[4].address(), 0x180107000);
  EXPECT_EQ(segments[4].size_in_memory(), 0x1041);

  EXPECT_EQ(segments[5].offset_in_file(), 0x103200);
  EXPECT_EQ(segments[5].size_in_file(), 0x200);
  EXPECT_EQ(segments[5].address(), 0x180109000);
  EXPECT_EQ(segments[5].size_in_memory(), 0x151);

  EXPECT_EQ(segments[6].offset_in_file(), 0x103400);
  EXPECT_EQ(segments[6].size_in_file(), 0x400);
  EXPECT_EQ(segments[6].address(), 0x18010A000);
  EXPECT_EQ(segments[6].size_in_memory(), 0x222);

  EXPECT_EQ(segments[7].offset_in_file(), 0x103800);
  EXPECT_EQ(segments[7].size_in_file(), 0x1C00);
  EXPECT_EQ(segments[7].address(), 0x18010B000);
  EXPECT_EQ(segments[7].size_in_memory(), 0x1A78);
}

}  // namespace orbit_object_utils
