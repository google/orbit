// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <vector>

#include "TestUtils/SaveRangeFromArg.h"

namespace orbit_test_utils {

TEST(SaveRangeFromArg, IntoVector) {
  std::vector<int> sink{};
  SaveRangeFromArg<0> (&sink)(absl::Span<int const>{1, 2, 3});
  EXPECT_THAT(sink, testing::ElementsAre(1, 2, 3));

  SaveRangeFromArg<1> (&sink)("unused", absl::Span<int const>{4, 5, 6});
  EXPECT_THAT(sink, testing::ElementsAre(4, 5, 6));
}

TEST(SaveRangeFromArg, IntoSet) {
  std::set<int> sink{};
  SaveRangeFromArg<0> (&sink)(absl::Span<int const>{1, 2, 3, 3, 2, 1});
  EXPECT_THAT(sink, testing::ElementsAre(1, 2, 3));

  SaveRangeFromArg<1> (&sink)("unused", absl::Span<int const>{4, 5, 6, 6, 6});
  EXPECT_THAT(sink, testing::ElementsAre(4, 5, 6));
}

}  // namespace orbit_test_utils