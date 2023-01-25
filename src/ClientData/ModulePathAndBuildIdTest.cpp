// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/hash/hash_testing.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <variant>
#include <vector>

#include "ClientData/ModulePathAndBuildId.h"

namespace orbit_client_data {
TEST(ModulePathAndBuildId, Hash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      ModulePathAndBuildId{.module_path = "/a/file/path", .build_id = "build_id"},
      ModulePathAndBuildId{.module_path = "a string", .build_id = "abcdefg"},
      ModulePathAndBuildId{.module_path = "", .build_id = ""},
      ModulePathAndBuildId{.module_path = "/a/file/path", .build_id = "build_id2"},
      ModulePathAndBuildId{.module_path = "", .build_id = "build_id"},
  }));
}

}  // namespace orbit_client_data
