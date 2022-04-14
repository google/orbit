// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "ClientData/ScopeInfo.h"
#include "absl/hash/hash_testing.h"

namespace orbit_client_data {
TEST(ScopeInfo, Hash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly(
      {ScopeInfo{}, ScopeInfo{"", ScopeType::kOther}, ScopeInfo{"", ScopeType::kHookedFunction},
       ScopeInfo{"", ScopeType::kApiScope}, ScopeInfo{"kapiscope", ScopeType::kApiScope},
       ScopeInfo{"kapiscope", ScopeType::kApiScopeAsync},
       ScopeInfo{"kApiScope", ScopeType::kApiScope},
       ScopeInfo{"kApiScope", ScopeType::kHookedFunction}}));
}
}  // namespace orbit_client_data