// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <outcome.hpp>
#include <string>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Result.h"
#include "Path.h"
#include "SymbolHelper.h"
#include "absl/strings/ascii.h"
#include "symbol.pb.h"

using orbit_grpc_protos::ModuleSymbols;
namespace fs = std::filesystem;

static const std::filesystem::path testdata_directory = orbit_base::GetExecutableDir() / "testdata";

TEST(SymbolHelper, FindSymbolsWithSymbolsPathFile) {
  SymbolHelper symbol_helper({testdata_directory}, "");
  {
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const fs::path symbols_path = testdata_directory / "no_symbols_elf.debug";

    const auto symbols_path_result = symbol_helper.FindSymbolsWithSymbolsPathFile(
        file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    ASSERT_TRUE(symbols_path_result) << symbols_path_result.error().message();
    EXPECT_EQ(symbols_path_result.value(), symbols_path);
  }

  {
    const fs::path non_existing_path = "file.not.exist";
    const auto symbols_path_result =
        symbol_helper.FindSymbolsWithSymbolsPathFile(non_existing_path, "irrelevant build id");
    ASSERT_FALSE(symbols_path_result);
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("could not find"));
  }

  {
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const auto symbols_path_result =
        symbol_helper.FindSymbolsWithSymbolsPathFile(file_path, "wrong build id");
    ASSERT_FALSE(symbols_path_result);
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("could not find"));
  }

  {
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const auto symbols_path_result = symbol_helper.FindSymbolsWithSymbolsPathFile(file_path, "");
    ASSERT_FALSE(symbols_path_result);
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("could not find"));
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("does not contain a build id"));
  }
}

TEST(SymbolHelper, FindSymbolsInCache) {
  SymbolHelper symbol_helper({}, testdata_directory);

  // This is more of a smoke test (looking for the same file)
  {
    const fs::path file = "no_symbols_elf.debug";
    const auto result =
        symbol_helper.FindSymbolsInCache(file, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    ASSERT_TRUE(result) << result.error().message();
    EXPECT_EQ(result.value(), testdata_directory / file);
  }

  // file in cache does not have symbols
  {
    const fs::path file_path = "no_symbols_elf";
    const auto result =
        symbol_helper.FindSymbolsInCache(file_path, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    ASSERT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("does not contain symbols"));
  }

  // file in cache has different build id
  {
    const fs::path file_path = "no_symbols_elf.debug";
    const auto result = symbol_helper.FindSymbolsInCache(file_path, "non matching build id");
    ASSERT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("has a different build id"));
  }
}

TEST(SymbolHelper, LoadFromFile) {
  // contains symbols
  {
    const fs::path file_path = testdata_directory / "no_symbols_elf.debug";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_TRUE(result) << result.error().message();
    const ModuleSymbols& symbols = result.value();

    EXPECT_EQ(symbols.symbols_file_path(), file_path);
    EXPECT_FALSE(symbols.symbol_infos().empty());
  }

  // does not contain symbols
  {
    const fs::path file_path = testdata_directory / "no_symbols_elf";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("does not have a .symtab section"));
  }

  // invalid file
  {
    const fs::path file_path = testdata_directory / "file_does_not_exist";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no such file or directory"));
  }
}

TEST(SymbolHelper, GenerateCachedFileName) {
  SymbolHelper symbol_helper{{}, Path::CreateOrGetCacheDir()};
  const std::filesystem::path file_path = "/var/data/filename.elf";
  const std::filesystem::path cache_file_path =
      Path::CreateOrGetCacheDir() / "_var_data_filename.elf";
  EXPECT_EQ(symbol_helper.GenerateCachedFileName(file_path), cache_file_path);
}

TEST(SymbolHelper, VerifySymbolsFile) {
  {
    // valid file containing symbols and matching build id
    fs::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const auto result = SymbolHelper::VerifySymbolsFile(symbols_file, build_id);
    EXPECT_TRUE(result);
  }
  {
    // valid file containing symbols, build id not matching;
    fs::path symbols_file = testdata_directory / "no_symbols_elf.debug";
    std::string build_id = "incorrect build id";
    const auto result = SymbolHelper::VerifySymbolsFile(symbols_file, build_id);
    EXPECT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("has a different build id"));
  }
  {
    // valid file no symbols and matching build id
    fs::path symbols_file = testdata_directory / "no_symbols_elf";
    std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const auto result = SymbolHelper::VerifySymbolsFile(symbols_file, build_id);
    EXPECT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("does not contain symbols"));
  }
  {
    // invalid file
    fs::path symbols_file = "path/to/invalid_file";
    std::string build_id = "build id does not matter";
    const auto result = SymbolHelper::VerifySymbolsFile(symbols_file, build_id);
    EXPECT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("unable to load elf file"));
  }
}

TEST(SymbolHelper, FindDebugInfoFileLocally) {
  SymbolHelper symbol_helper({testdata_directory}, "");
  constexpr uint32_t kExpectedChecksum = 0x2bf887bf;

  const auto symbols_path_result =
      symbol_helper.FindDebugInfoFileLocally("hello_world_elf.debug", kExpectedChecksum);
  ASSERT_TRUE(symbols_path_result) << symbols_path_result.error().message();
  EXPECT_EQ(symbols_path_result.value().filename(), "hello_world_elf.debug");
  EXPECT_EQ(symbols_path_result.value().parent_path(), testdata_directory);
}

TEST(SymbolHelper, IsMatchingDebugInfoFile) {
  constexpr uint32_t kExpectedChecksum = 0x2bf887bf;
  auto correct_file_path = testdata_directory / "hello_world_elf.debug";
  auto existing_but_wrong_file_path = testdata_directory / "hello_world_elf";
  auto non_existing_file_path = testdata_directory / "hello_world_elf.does_not_exist";
  EXPECT_TRUE(SymbolHelper::IsMatchingDebugInfoFile(correct_file_path, kExpectedChecksum));
  EXPECT_FALSE(
      SymbolHelper::IsMatchingDebugInfoFile(existing_but_wrong_file_path, kExpectedChecksum));
  EXPECT_FALSE(SymbolHelper::IsMatchingDebugInfoFile(non_existing_file_path, kExpectedChecksum));
}