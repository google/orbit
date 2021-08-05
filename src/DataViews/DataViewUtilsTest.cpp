// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/time/time.h>
#include <gtest/gtest.h>

#include "DataViewUtils.h"

namespace orbit_data_views {

TEST(DataViewUtils, FormatShortDatetime) {
  absl::Time datetime = absl::FromTimeT(0);
  std::string result = FormatShortDatetime(datetime);
  EXPECT_EQ(result, "01/01/1970 00:00 AM");
}

}  // namespace orbit_data_views