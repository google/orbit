// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <memory>
#include <optional>
#include <string_view>

#include "OrbitBase/Result.h"
#include "SshQtTestUtils/ParsePortNumberFromSocatOutput.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh_qt_test_utils {

constexpr std::string_view kReferenceSocatOutput =
    "2022/12/12 12:12:42 socat[394] N listening on AF=2 0.0.0.0:58747\n";

TEST(ParsePortNumberFromSocatOutput, IncompleteInput) {
  auto result = ParsePortNumberFromSocatOutput(
      kReferenceSocatOutput.substr(0, kReferenceSocatOutput.size() - 1));
  EXPECT_FALSE(result.has_value());
}

TEST(ParsePortNumberFromSocatOutput, CompleteInput) {
  auto result = ParsePortNumberFromSocatOutput(kReferenceSocatOutput);
  EXPECT_TRUE(result.has_value());
  EXPECT_THAT(result.value(), orbit_test_utils::HasValue(58747));
}

TEST(ParsePortNumberFromSocatOutput, InvalidInput) {
  constexpr std::array kInvalidSocatOutputs{
      std::string_view{
          "2022/12/12 12:12:42 socat[394] N listening on AF=2 0.0.0.0:noport\n"},  // invalid port
      std::string_view{
          "2022/12/12 12:12:42 socat[394] N listening on AF=2 0.0.0.058747\n"}};  // Missing colon

  for (const auto invalid_input : kInvalidSocatOutputs) {
    auto result = ParsePortNumberFromSocatOutput(invalid_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->has_error());
  }
}
}  // namespace orbit_ssh_qt_test_utils