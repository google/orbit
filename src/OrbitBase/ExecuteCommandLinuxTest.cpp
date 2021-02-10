// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "OrbitBase/ExecuteCommand.h"

namespace orbit_base {
namespace {

TEST(ExecuteCommand, EchoHelloWorld) {
  std::string string_to_echo = "Hello, World!";
  std::optional<std::string> returned_result =
      ExecuteCommand(absl::StrFormat("echo %s", string_to_echo));
  std::string expected_result = string_to_echo + "\n";
  ASSERT_TRUE(returned_result.has_value());
  EXPECT_EQ(returned_result.value(), expected_result);
}

}  // namespace
}  // namespace orbit_base
