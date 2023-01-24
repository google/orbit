// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <string>

#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/StopSource.h"
#include "SymbolProvider/StructuredDebugDirectorySymbolProvider.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

using orbit_test_utils::HasValue;

namespace orbit_symbol_provider {

constexpr SymbolLoadingSuccessResult::SymbolSource kSymbolSource =
    SymbolLoadingSuccessResult::SymbolSource::kStadiaSymbolStore;

class StructuredDebugDirectorySymbolProviderTest : public ::testing::Test {
 public:
  explicit StructuredDebugDirectorySymbolProviderTest()
      : symbol_provider_(orbit_test::GetTestdataDir() / "debugstore", kSymbolSource) {}

 protected:
  StructuredDebugDirectorySymbolProvider symbol_provider_;
  orbit_base::StopSource stop_source_;
};

TEST_F(StructuredDebugDirectorySymbolProviderTest, RetrieveSymbolsSuccessfully) {
  const std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";

  const orbit_base::Future<SymbolLoadingOutcome> future = symbol_provider_.RetrieveSymbols(
      {.module_path = "/not/needed/module/path", .build_id = build_id},
      stop_source_.GetStopToken());

  bool lambda_executed = false;
  orbit_base::ImmediateExecutor executor;
  future
      .Then(&executor,
            [&](const SymbolLoadingOutcome& result) {
              ASSERT_THAT(result, HasValue());
              ASSERT_TRUE(IsSuccessResult(result));
              SymbolLoadingSuccessResult success_result = GetSuccessResult(result);
              const std::filesystem::path symbol_path =
                  orbit_test::GetTestdataDir() / "debugstore" / ".build-id" / "b5" /
                  "413574bbacec6eacb3b89b1012d0e2cd92ec6b.debug";
              EXPECT_EQ(success_result.path, symbol_path);
              EXPECT_EQ(success_result.symbol_file_separation,
                        SymbolLoadingSuccessResult::SymbolFileSeparation::kDifferentFile);
              EXPECT_EQ(success_result.symbol_source, kSymbolSource);
              lambda_executed = true;
            })
      .Wait();
  EXPECT_TRUE(lambda_executed);
}

TEST_F(StructuredDebugDirectorySymbolProviderTest, RetrieveSymbolsNotFound) {
  const std::string build_id = "different build id";

  const orbit_base::Future<SymbolLoadingOutcome> future = symbol_provider_.RetrieveSymbols(
      {.module_path = "/not/needed/module/path", .build_id = build_id},
      stop_source_.GetStopToken());

  bool lambda_executed = false;
  orbit_base::ImmediateExecutor executor;
  future
      .Then(&executor,
            [&](const SymbolLoadingOutcome& result) {
              ASSERT_THAT(result, HasValue());
              ASSERT_TRUE(IsNotFound(result));
              const std::string& not_found_message = GetNotFoundMessage(result);
              EXPECT_TRUE(absl::StrContains(not_found_message, "File does not exist"));
              lambda_executed = true;
            })
      .Wait();
  EXPECT_TRUE(lambda_executed);
}

TEST_F(StructuredDebugDirectorySymbolProviderTest, RetrieveSymbolsError) {
  {
    const std::string build_id = "a";  // build id mal formed (too short)

    const orbit_base::Future<SymbolLoadingOutcome> future = symbol_provider_.RetrieveSymbols(
        {.module_path = "/not/needed/module/path", .build_id = build_id},
        stop_source_.GetStopToken());

    bool lambda_executed = false;
    orbit_base::ImmediateExecutor executor;
    future
        .Then(&executor,
              [&](const SymbolLoadingOutcome& result) {
                ASSERT_THAT(result, orbit_test_utils::HasErrorWithMessage("malformed"));
                lambda_executed = true;
              })
        .Wait();
    EXPECT_TRUE(lambda_executed);
  }
}

}  // namespace orbit_symbol_provider