// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "OrbitBase/StopSource.h"
#include "OrbitBase/StopToken.h"

namespace orbit_base {

TEST(StopSource, ConstructStopPossible) {
  StopSource stop_source{};
  EXPECT_TRUE(stop_source.IsStopPossible());
}

TEST(StopSource, MoveStopPossible) {
  {
    StopSource stop_source{};
    StopSource stop_source_moved{std::move(stop_source)};
    EXPECT_FALSE(stop_source.IsStopPossible());
    EXPECT_TRUE(stop_source_moved.IsStopPossible());
  }
  {
    StopSource stop_source{};
    StopSource stop_source_moved = std::move(stop_source);
    EXPECT_FALSE(stop_source.IsStopPossible());
    EXPECT_TRUE(stop_source_moved.IsStopPossible());
  }
}

TEST(StopSource, RequestStop) {
  StopSource stop_source{};

  StopToken stop_token_1 = stop_source.GetStopToken();
  ASSERT_TRUE(stop_source.IsStopPossible());
  EXPECT_FALSE(stop_token_1.IsStopRequested());

  StopToken stop_token_2 = stop_source.GetStopToken();
  ASSERT_TRUE(stop_source.IsStopPossible());
  EXPECT_FALSE(stop_token_2.IsStopRequested());

  stop_source.RequestStop();
  EXPECT_TRUE(stop_source.IsStopPossible());

  ASSERT_TRUE(stop_token_1.IsStopPossible());
  EXPECT_TRUE(stop_token_1.IsStopRequested());

  ASSERT_TRUE(stop_token_2.IsStopPossible());
  EXPECT_TRUE(stop_token_2.IsStopRequested());
}

TEST(StopSource, MoveAndRequestStop) {
  StopSource stop_source{};
  StopToken stop_token = stop_source.GetStopToken();

  StopSource stop_source_moved{std::move(stop_source)};

  StopToken stop_token_from_moved = stop_source_moved.GetStopToken();
  EXPECT_FALSE(stop_token_from_moved.IsStopRequested());

  // Old stop tokens are still valid and connected
  ASSERT_TRUE(stop_token.IsStopPossible());
  EXPECT_EQ(stop_token.IsStopRequested(), stop_token_from_moved.IsStopRequested());

  stop_source_moved.RequestStop();

  EXPECT_TRUE(stop_token_from_moved.IsStopRequested());
  EXPECT_EQ(stop_token.IsStopRequested(), stop_token_from_moved.IsStopRequested());
}

}  // namespace orbit_base