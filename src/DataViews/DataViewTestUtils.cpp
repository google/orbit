// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViewTestUtils.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace orbit_data_views {

void CheckSingleAction(const std::vector<std::string>& context_menu, const std::string& action,
                       bool enable_action) {
  if (enable_action) {
    EXPECT_THAT(context_menu, testing::IsSupersetOf({action}));
  } else {
    EXPECT_THAT(context_menu, testing::Not(testing::Contains(action)));
  }
}

}  // namespace orbit_data_views