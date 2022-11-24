// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>

#include "OrbitVersion/OrbitVersion.h"

namespace orbit_version {

TEST(OrbitVersion, Compare) {
  EXPECT_TRUE((Version{1, 1} == Version{1, 1}));
  EXPECT_FALSE((Version{1, 1} == Version{1, 2}));
  EXPECT_FALSE((Version{2, 1} == Version{1, 1}));

  EXPECT_FALSE((Version{1, 1} != Version{1, 1}));
  EXPECT_TRUE((Version{1, 1} != Version{1, 2}));
  EXPECT_TRUE((Version{2, 1} != Version{1, 1}));

  EXPECT_TRUE((Version{1, 1} < Version{1, 2}));
  EXPECT_TRUE((Version{1, 2} < Version{2, 1}));
  EXPECT_FALSE((Version{1, 1} < Version{1, 1}));
  EXPECT_FALSE((Version{2, 1} < Version{1, 2}));

  EXPECT_TRUE((Version{1, 1} <= Version{1, 2}));
  EXPECT_TRUE((Version{1, 2} <= Version{2, 1}));
  EXPECT_TRUE((Version{1, 1} <= Version{1, 1}));
  EXPECT_FALSE((Version{2, 1} <= Version{1, 2}));

  EXPECT_TRUE((Version{1, 2} > Version{1, 1}));
  EXPECT_TRUE((Version{2, 1} > Version{1, 2}));
  EXPECT_FALSE((Version{1, 1} > Version{1, 1}));
  EXPECT_FALSE((Version{1, 2} > Version{2, 1}));

  EXPECT_TRUE((Version{1, 2} >= Version{1, 1}));
  EXPECT_TRUE((Version{2, 1} >= Version{1, 2}));
  EXPECT_TRUE((Version{1, 1} >= Version{1, 1}));
  EXPECT_FALSE((Version{1, 2} >= Version{2, 1}));
}

TEST(OrbitVersion, MajorVersionIsAlwaysOne) { EXPECT_EQ(GetVersion().major_version, 1); }

}  // namespace orbit_version