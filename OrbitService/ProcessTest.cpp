// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>

#include "ProcessList.h"
#include "gtest/gtest.h"

constexpr int kTaskCommLength = 16;

namespace orbit_service {

TEST(Process, FromPid) {
  auto potential_process = Process::FromPid(getpid());
  ASSERT_TRUE(potential_process);

  auto& process = potential_process.value();

  EXPECT_EQ(process.pid(), getpid());
  EXPECT_EQ(process.name(), std::string_view{"OrbitServiceTests"}.substr(0, kTaskCommLength - 1));
}

}  // namespace orbit_service
