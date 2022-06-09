// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <variant>

#include "OrbitBase/CanceledOr.h"

namespace orbit_base {

TEST(CanceledOr, IsCanceled) {
  // Default constructed is NOT canceled
  CanceledOr<int> canceled_or_int;
  EXPECT_FALSE(IsCanceled(canceled_or_int));

  canceled_or_int = Canceled{};
  EXPECT_TRUE(IsCanceled(canceled_or_int));

  canceled_or_int = 5;
  EXPECT_FALSE(IsCanceled(canceled_or_int));

  // Default constructed is NOT canceled
  CanceledOr<void> canceled_or_void;
  EXPECT_FALSE(IsCanceled(canceled_or_void));

  canceled_or_void = Canceled{};
  EXPECT_TRUE(IsCanceled(canceled_or_void));
}

}  // namespace orbit_base