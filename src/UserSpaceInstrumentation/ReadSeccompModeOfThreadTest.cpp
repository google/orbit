// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <linux/seccomp.h>
#include <unistd.h>

#include <memory>
#include <optional>

#include "ReadSeccompModeOfThread.h"

namespace orbit_user_space_instrumentation {

TEST(ReadSeccompModeOfThread, ReadSeccompModeOfThread) {
  std::optional<int> seccomp_mode = ReadSeccompModeOfThread(getpid());
  ASSERT_TRUE(seccomp_mode.has_value());
  EXPECT_EQ(seccomp_mode.value(), SECCOMP_MODE_DISABLED);
}

}  // namespace orbit_user_space_instrumentation
