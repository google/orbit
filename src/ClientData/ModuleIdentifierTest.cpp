// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/hash/hash_testing.h>
#include <gtest/gtest.h>

#include "ClientData/ModuleIdentifier.h"

namespace orbit_client_data {

TEST(ModuleIdentifier, Hash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      ModuleIdentifier{},
      ModuleIdentifier{"/abc", "1234"},
      ModuleIdentifier{"/abc", "1235"},
      ModuleIdentifier{"/abd", "1234"},
      ModuleIdentifier{"/abd", "1235"},
  }));
}

}  // namespace orbit_client_data