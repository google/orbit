// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <memory>

#include "Path.h"
#include "SymbolHelper.h"
#include "absl/strings/ascii.h"
#include "symbol.pb.h"

using orbit_grpc_protos::ModuleSymbols;
namespace fs = std::filesystem;

const fs::path executable_directory = Path::GetExecutablePath() + "testdata/";

TEST(SymbolHelper, FindSymbolsWithSymbolsPathFile) {
  SymbolHelper symbol_helper({executable_directory}, "");
  {
    const fs::path file_path = executable_directory / "no_symbols_elf";
    const fs::path symbols_path = executable_directory / "no_symbols_elf.debug";

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
    const fs::path file_path = executable_directory / "no_symbols_elf";
    const auto symbols_path_result =
        symbol_helper.FindSymbolsWithSymbolsPathFile(file_path, "wrong build id");
    ASSERT_FALSE(symbols_path_result);
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("could not find"));
  }

  {
    const fs::path file_path = executable_directory / "no_symbols_elf";
    const auto symbols_path_result = symbol_helper.FindSymbolsWithSymbolsPathFile(file_path, "");
    ASSERT_FALSE(symbols_path_result);
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("could not find"));
    EXPECT_THAT(absl::AsciiStrToLower(symbols_path_result.error().message()),
                testing::HasSubstr("does not contain a build id"));
  }
}

TEST(SymbolHelper, FindSymbolsInCache) {
  SymbolHelper symbol_helper({}, executable_directory);

  // This is more of a smoke test (looking for the same file)
  {
    const fs::path file = "no_symbols_elf.debug";
    const auto result =
        symbol_helper.FindSymbolsInCache(file, "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
    ASSERT_TRUE(result) << result.error().message();
    EXPECT_EQ(result.value(), executable_directory / file);
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
    const fs::path file_path = executable_directory / "no_symbols_elf.debug";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_TRUE(result) << result.error().message();
    const ModuleSymbols& symbols = result.value();

    EXPECT_EQ(symbols.symbols_file_path(), file_path);
    EXPECT_FALSE(symbols.symbol_infos().empty());
  }

  // does not contain symbols
  {
    const fs::path file_path = executable_directory / "no_symbols_elf";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("does not have a .symtab section"));
  }

  // invalid file
  {
    const fs::path file_path = executable_directory / "file_does_not_exist";
    const auto result = SymbolHelper::LoadSymbolsFromFile(file_path);

    ASSERT_FALSE(result);
    EXPECT_THAT(absl::AsciiStrToLower(result.error().message()),
                testing::HasSubstr("no such file or directory"));
  }
}

TEST(SymbolHelper, GenerateCachedFileName) {
  SymbolHelper symbol_helper{{}, Path::GetCachePath()};
  const std::string file_path = "/var/data/filename.elf";
  const std::string cache_file_path = Path::GetCachePath() + "/_var_data_filename.elf";
  EXPECT_EQ(symbol_helper.GenerateCachedFileName(file_path), cache_file_path);
}