// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/hash/hash_testing.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <variant>
#include <vector>

#include "ClientData/ModuleIdentifier.h"
#include "ClientData/ModuleIdentifierProvider.h"

namespace orbit_client_data {

TEST(ModuleIdentifier, Hash) {
  ModuleIdentifierProvider module_identifier_provider;
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      module_identifier_provider.CreateModuleIdentifier(
          {.module_path = "/a/file/path", .build_id = "build_id"}),
      module_identifier_provider.CreateModuleIdentifier(
          {.module_path = "a string", .build_id = "abcdefg"}),
      module_identifier_provider.CreateModuleIdentifier({.module_path = "", .build_id = ""}),
      module_identifier_provider.CreateModuleIdentifier(
          {.module_path = "/a/file/path", .build_id = "build_id2"}),
      module_identifier_provider.CreateModuleIdentifier(
          {.module_path = "", .build_id = "build_id"}),
  }));
}

}  // namespace orbit_client_data