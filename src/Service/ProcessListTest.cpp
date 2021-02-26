// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <gtest/gtest.h>
#include <unistd.h>

#include <memory>
#include <optional>
#include <outcome.hpp>

#include "OrbitBase/Result.h"
#include "ProcessList.h"
#include "ServiceUtils.h"
#include "gtest/gtest.h"

namespace orbit_service {

TEST(ProcessList, ProcessList) {
  ProcessList process_list;
  const auto result1 = process_list.Refresh();
  ASSERT_TRUE(result1.has_value()) << result1.error().message();

  const auto process1 = process_list.GetProcessByPid(getpid());
  EXPECT_TRUE(process1.has_value());

  const auto total_cpu_cycles = utils::GetCumulativeTotalCpuTime().value();

  // We wait until the stats have been updated
  while (utils::GetCumulativeTotalCpuTime().value().jiffies.value ==
         total_cpu_cycles.jiffies.value) {
    // If this loop never ends it will be caught by the automatic timeout feature
    usleep(10'000);
  }

  const auto result2 = process_list.Refresh();
  ASSERT_TRUE(result2.has_value()) << result2.error().message();

  const auto process2 = process_list.GetProcessByPid(getpid());
  EXPECT_TRUE(process2.has_value());
}

}  // namespace orbit_service
