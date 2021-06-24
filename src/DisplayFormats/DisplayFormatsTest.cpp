// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>

#include "DisplayFormats/DisplayFormats.h"

namespace orbit_display_formats {

TEST(DisplayFormats, GetDisplaySize) {
  EXPECT_EQ(GetDisplaySize(123), "123 B");
  EXPECT_EQ(GetDisplaySize(123 * 1024ULL + 512 + 256), "123.75 KB");
  EXPECT_EQ(GetDisplaySize(1024 * (123 * 1024ULL + 512 + 256)), "123.75 MB");
  EXPECT_EQ(GetDisplaySize(1024 * 1024 * (123 * 1024ULL + 512 + 256)), "123.75 GB");
  EXPECT_EQ(GetDisplaySize(1024 * 1024 * 1024 * (123 * 1024ULL + 512 + 256)), "123.75 TB");
}

TEST(DisplayFormats, GetDisplayTime) {
  EXPECT_EQ(GetDisplayTime(absl::Nanoseconds(12)), "12.000 ns");
  EXPECT_EQ(GetDisplayTime(absl::Nanoseconds(12'345)), "12.345 us");
  EXPECT_EQ(GetDisplayTime(absl::Nanoseconds(12'345'600)), "12.346 ms");
  EXPECT_EQ(GetDisplayTime(absl::Nanoseconds(12'345'600'000ULL)), "12.346 s");
  EXPECT_EQ(GetDisplayTime(absl::Nanoseconds(60 * 12'345'600'000ULL)), "12.346 min");
  EXPECT_EQ(GetDisplayTime(absl::Nanoseconds(60 * 60 * 12'345'600'000ULL)), "12.346 h");
  EXPECT_EQ(GetDisplayTime(absl::Nanoseconds(24 * 60 * 60 * 12'345'600'000ULL)), "12.346 days");
}

}  // namespace orbit_display_formats