// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/hash/hash_testing.h>
#include <gtest/gtest.h>

#include <initializer_list>
#include <variant>
#include <vector>

#include "ClientData/ScopeInfo.h"

namespace orbit_client_data {
TEST(ScopeInfo, Hash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly(
      {ScopeInfo{"", ScopeType::kInvalid},
       ScopeInfo{"", ScopeType::kDynamicallyInstrumentedFunction},
       ScopeInfo{"", ScopeType::kApiScope}, ScopeInfo{"kapiscope", ScopeType::kApiScope},
       ScopeInfo{"kapiscope", ScopeType::kApiScopeAsync},
       ScopeInfo{"kApiScope", ScopeType::kApiScope},
       ScopeInfo{"kApiScope", ScopeType::kDynamicallyInstrumentedFunction}}));
}
}  // namespace orbit_client_data