// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/Align.h"

namespace orbit_base {

TEST(Align, AlignUpSmoke) {
  EXPECT_EQ(AlignUp<4>(0), 0);
  EXPECT_EQ(AlignUp<4>(1), 4);
  EXPECT_EQ(AlignUp<4>(3), 4);
  EXPECT_EQ(AlignUp<4>(4), 4);
  EXPECT_EQ(AlignUp<16>(17), 32);
}

}  // namespace orbit_base