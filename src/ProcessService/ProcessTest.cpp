// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>

#include "GrpcProtos/process.pb.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProcessService/Process.h"

namespace orbit_process_service_internal {

TEST(Process, FromPid) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  auto potential_process = Process::FromPid(pid);
  ASSERT_TRUE(potential_process.has_value()) << potential_process.error().message();

  auto& process = potential_process.value();

  EXPECT_EQ(process.process_info().pid(), pid);
  EXPECT_EQ(process.process_info().name(), orbit_base::GetThreadName(pid));
}

}  // namespace orbit_process_service_internal
