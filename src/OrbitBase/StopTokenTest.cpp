// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "OrbitBase/Future.h"
#include "OrbitBase/StopSource.h"
#include "OrbitBase/StopToken.h"

namespace orbit_base {

class StopTokenTest : public testing::Test {
 protected:
  StopTokenTest() : stop_token_(stop_source_.GetStopToken()) {}

  void RequestStop() { stop_source_.RequestStop(); }

 private:
  StopSource stop_source_;

 protected:
  StopToken stop_token_;
};

TEST_F(StopTokenTest, RequestStop) {
  ASSERT_TRUE(stop_token_.IsStopPossible());
  EXPECT_FALSE(stop_token_.IsStopRequested());
  RequestStop();
  ASSERT_TRUE(stop_token_.IsStopPossible());
  EXPECT_TRUE(stop_token_.IsStopRequested());
}

TEST_F(StopTokenTest, Copy) {
  StopToken stop_token_copy{stop_token_};
  ASSERT_TRUE(stop_token_copy.IsStopPossible());
  EXPECT_FALSE(stop_token_copy.IsStopRequested());

  RequestStop();
  ASSERT_TRUE(stop_token_copy.IsStopPossible());
  EXPECT_TRUE(stop_token_copy.IsStopRequested());
}

TEST_F(StopTokenTest, Move) {
  StopToken moved_stop_token{std::move(stop_token_)};

  ASSERT_TRUE(moved_stop_token.IsStopPossible());
  EXPECT_FALSE(moved_stop_token.IsStopRequested());

  RequestStop();
  ASSERT_TRUE(moved_stop_token.IsStopPossible());
  EXPECT_TRUE(moved_stop_token.IsStopRequested());

  // Moved from token is not valid anymore.
  EXPECT_FALSE(stop_token_.IsStopPossible());
}

TEST_F(StopTokenTest, GetFuture) {
  Future<void> future = stop_token_.GetFuture();

  EXPECT_TRUE(future.IsValid());
  EXPECT_FALSE(future.IsFinished());

  RequestStop();
  EXPECT_TRUE(future.IsFinished());
}
}  // namespace orbit_base