// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest-death-test.h>
#include <gtest/gtest.h>

#include <memory>

#include "OrbitBase/SharedState.h"
#include "OrbitBase/StopToken.h"

namespace orbit_base {

TEST(StopToken, Constructor) {
  StopToken stop_token{};
  EXPECT_FALSE(stop_token.IsStopPossible());
  EXPECT_DEATH((void)stop_token.IsStopRequested(), "");
}

TEST(StopToken, Copy) {
  {  // ctor
    StopToken stop_token{};

    StopToken stop_token_copy{stop_token};
    EXPECT_FALSE(stop_token_copy.IsStopPossible());
  }
  {  // assignment
    StopToken stop_token{};
    EXPECT_FALSE(stop_token.IsStopPossible());

    StopToken stop_token_copy = stop_token;
    EXPECT_FALSE(stop_token_copy.IsStopPossible());
  }
}

TEST(StopToken, Move) {
  {  // ctor
    StopToken stop_token{};

    StopToken stop_token_moved{std::move(stop_token)};
    EXPECT_FALSE(stop_token_moved.IsStopPossible());
  }
  {  // assignment
    StopToken stop_token{};

    StopToken stop_token_moved = std::move(stop_token);
    EXPECT_FALSE(stop_token_moved.IsStopPossible());
  }
}

TEST(StopToken, InvalidAccess) {
  StopToken stop_token{};
  EXPECT_FALSE(stop_token.IsStopPossible());
  EXPECT_DEATH((void)stop_token.IsStopRequested(), "");
}

}  // namespace orbit_base