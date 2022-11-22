// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>

#include "OrbitGl/ShortenStringWithEllipsis.h"

namespace orbit_gl {

TEST(ShortenStringWithEllipsis, ShortenStringWithEllipsis) {
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 18), "17 char long text");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 17), "17 char long text");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 7), "17...xt");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 6), "17...t");
  EXPECT_EQ(ShortenStringWithEllipsis("short", 4), "s...");
  EXPECT_EQ(ShortenStringWithEllipsis("short", 3), "...");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 2), "...");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 1), "...");
  EXPECT_EQ(ShortenStringWithEllipsis("17 char long text", 0), "...");
  // Texts with 3 or less characters are not shortened as it makes no sense
  EXPECT_EQ(ShortenStringWithEllipsis("abc", 2), "abc");
  EXPECT_EQ(ShortenStringWithEllipsis("abc", 1), "abc");
  EXPECT_EQ(ShortenStringWithEllipsis("abc", 0), "abc");
}

}  // namespace orbit_gl
