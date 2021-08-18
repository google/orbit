// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <unistd.h>

#include <memory>
#include <string_view>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Result.h"
#include "Process.h"

constexpr size_t kTaskCommLength = 16;

namespace orbit_service {

TEST(Process, FromPid) {
  auto potential_process = Process::FromPid(getpid());
  ASSERT_TRUE(potential_process.has_value()) << potential_process.error().message();

  auto& process = potential_process.value();

  EXPECT_EQ(process.process_info().pid(), getpid());
  EXPECT_EQ(process.process_info().name(),
            orbit_base::GetExecutablePath().filename().string().substr(0, kTaskCommLength - 1));
}

}  // namespace orbit_service
