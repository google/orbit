// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <unistd.h>

#include "OrbitBase/Logging.h"
#include "ProcessList.h"
#include "gtest/gtest.h"

namespace orbit_service {

TEST(ProcessList, ProcessList) {
  ProcessList process_list;
  const auto result1 = process_list.Refresh();
  ASSERT_TRUE(result1) << result1.error().message();

  const auto process1 = process_list.GetProcessByPid(getpid());
  EXPECT_TRUE(process1);

  const auto total_cpu_cycles = utils::GetCumulativeTotalCpuTime().value();

  // We wait until the stats have been updated
  while (utils::GetCumulativeTotalCpuTime().value().value == total_cpu_cycles.value) {
    // If this loop never ends it will be caught by the automatic timeout feature
    usleep(10'000);
  }

  const auto result2 = process_list.Refresh();
  ASSERT_TRUE(result2) << result2.error().message();

  const auto process2 = process_list.GetProcessByPid(getpid());
  EXPECT_TRUE(process2);
}

}  // namespace orbit_service
