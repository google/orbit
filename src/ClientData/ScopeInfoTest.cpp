// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/ScopeInfo.h"
#include "absl/hash/hash_testing.h"

namespace orbit_client_data {
TEST(ScopeInfo, Hash) {
  EXPECT_TRUE(
      absl::VerifyTypeImplementsAbslHashCorrectly({ScopeInfo{},
                                                   {"", ScopeType::kOther},
                                                   {"", ScopeType::kHookedFunction},
                                                   {"", ScopeType::kApiScope},
                                                   {"kapiscope", ScopeType::kApiScope},
                                                   {"kapiscope", ScopeType::kApiScopeAsync},
                                                   {"kApiScope", ScopeType::kApiScope},
                                                   {"kApiScope", ScopeType::kHookedFunction}}));
}
}  // namespace orbit_client_data