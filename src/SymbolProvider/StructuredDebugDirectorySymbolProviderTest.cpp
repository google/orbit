// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
#include <gtest/gtest.h>

#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/StopSource.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SymbolProvider/StructuredDebugDirectorySymbolProvider.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

using orbit_test_utils::HasValue;

namespace orbit_symbol_provider {

constexpr SymbolLoadingSuccessResult::SymbolSource kSymbolSource =
    SymbolLoadingSuccessResult::SymbolSource::kStadiaSymbolStore;

TEST(StructuredDebugDirectorySymbolProvider, RetrieveSymbols) {
  const std::filesystem::path testdata_directory = orbit_test::GetTestdataDir();
  const std::filesystem::path symbols_path = testdata_directory / "debugstore" / ".build-id" /
                                             "b5" / "413574bbacec6eacb3b89b1012d0e2cd92ec6b.debug";
  const orbit_base::StopSource stop_source;

  StructuredDebugDirectorySymbolProvider symbol_provider{testdata_directory / "debugstore",
                                                         kSymbolSource};

  // Test 1: Success
  {
    const std::string build_id = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";
    const ModuleIdentifier module_id{"/not/needed/module/path", build_id};

    const orbit_base::Future<SymbolLoadingOutcome> future =
        symbol_provider.RetrieveSymbols(module_id, stop_source.GetStopToken());

    bool lambda_executed = false;
    orbit_base::ImmediateExecutor executor;
    future
        .Then(&executor,
              [&](const SymbolLoadingOutcome& result) {
                ASSERT_THAT(result, HasValue());
                ASSERT_TRUE(IsSuccessResult(result));
                SymbolLoadingSuccessResult success_result = GetSuccessResult(result);
                EXPECT_EQ(success_result.path, symbols_path);
                EXPECT_EQ(success_result.symbol_file_separation,
                          SymbolLoadingSuccessResult::SymbolFileSeparation::kDifferentFile);
                EXPECT_EQ(success_result.symbol_source, kSymbolSource);
                lambda_executed = true;
              })
        .Wait();
    EXPECT_TRUE(lambda_executed);
  }

  // Test 2: Not found
  {
    const std::string build_id = "different build id";
    const ModuleIdentifier module_id{"/not/needed/module/path", build_id};

    const orbit_base::Future<SymbolLoadingOutcome> future =
        symbol_provider.RetrieveSymbols(module_id, stop_source.GetStopToken());

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

  // Test 3: Error
  {
    const std::string build_id = "a";  // build id mal formed (too short)
    const ModuleIdentifier module_id{"/not/needed/module/path", build_id};

    const orbit_base::Future<SymbolLoadingOutcome> future =
        symbol_provider.RetrieveSymbols(module_id, stop_source.GetStopToken());

    bool lambda_executed = false;
    orbit_base::ImmediateExecutor executor;
    future
        .Then(&executor,
              [&](const SymbolLoadingOutcome& result) {
                ASSERT_THAT(result, orbit_test_utils::HasError("malformed"));
                lambda_executed = true;
              })
        .Wait();
    EXPECT_TRUE(lambda_executed);
  }
}

}  // namespace orbit_symbol_provider