// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <vector>

#include "ObjectUtils/CoffFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/TestUtils.h"
#include "absl/strings/ascii.h"
#include "symbol.pb.h"

using orbit_base::HasNoError;
using orbit_grpc_protos::SymbolInfo;
using orbit_object_utils::CoffFile;
using orbit_object_utils::CreateCoffFile;

TEST(CoffFile, LoadDebugSymbols) {
  std::filesystem::path file_path = orbit_base::GetExecutableDir() / "testdata" / "libtest.dll";

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
  EXPECT_EQ(symbol_info.address(), 0x1000);
  // COFF symbols do not have a size, we expect 0 in this case.
  EXPECT_EQ(symbol_info.size(), 0xc);

  symbol_info = symbol_infos[5];
  EXPECT_EQ(symbol_info.name(), "PrintHelloWorld");
  EXPECT_EQ(symbol_info.demangled_name(), "PrintHelloWorld");
  EXPECT_EQ(symbol_info.address(), 0x13a0);
  // COFF symbols do not have a size, we expect 0 in this case.
  EXPECT_EQ(symbol_info.size(), 0x1b);
}

TEST(CoffFile, HasDebugSymbols) {
  std::filesystem::path file_path = orbit_base::GetExecutableDir() / "testdata" / "libtest.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());

  EXPECT_TRUE(coff_file_result.value()->HasDebugSymbols());
}

TEST(CoffFile, GetFilePath) {
  std::filesystem::path file_path = orbit_base::GetExecutableDir() / "testdata" / "libtest.dll";

  auto coff_file_result = CreateCoffFile(file_path);
  ASSERT_THAT(coff_file_result, HasNoError());

  EXPECT_EQ(coff_file_result.value()->GetFilePath(), file_path);
}

TEST(CoffFile, FileDoesNotExist) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "does_not_exist";

  auto coff_file_or_error = CreateCoffFile(file_path);
  ASSERT_TRUE(coff_file_or_error.has_error());
  EXPECT_THAT(absl::AsciiStrToLower(coff_file_or_error.error().message()),
              testing::HasSubstr("no such file or directory"));
}
