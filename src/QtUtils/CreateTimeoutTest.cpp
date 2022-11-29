// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gtest/gtest.h>

#include <QEventLoop>
#include <algorithm>
#include <array>
#include <chrono>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/WhenAll.h"
#include "QtUtils/CreateTimeout.h"

namespace orbit_qt_utils {

template <typename T>
static void WaitForFutureToComplete(const orbit_base::Future<T>& future) {
  QEventLoop loop{};
  orbit_base::ImmediateExecutor executor{};
  future.Then(&executor, [&loop](const T&) { loop.quit(); });
  loop.exec();
}

TEST(CreateTimeout, TimeoutCompletesEventually) {
  for (int i = 0; i < 10; ++i) {
    orbit_base::Future<Timeout> timeout = CreateTimeout(std::chrono::milliseconds{10 + i});
    WaitForFutureToComplete(timeout);
  }

  SUCCEED();
}

TEST(CreateTimeout, ParallelTimeoutsDontDeadlock) {
  std::vector<orbit_base::Future<Timeout>> timeouts;
  for (int i = 0; i < 10; ++i) {
    timeouts.emplace_back(CreateTimeout(std::chrono::milliseconds{10 + i}));
  }

  WaitForFutureToComplete(orbit_base::WhenAll(absl::MakeConstSpan(timeouts)));

  SUCCEED();
}

TEST(CreateTimeout, MaintainsMinimumWaitTime) {
  using namespace std::chrono_literals;
  std::array<std::chrono::milliseconds, 10> durations = {1ms,  2ms,  5ms,  10ms, 15ms,
                                                         20ms, 25ms, 30ms, 40ms, 100ms};

  for (std::chrono::milliseconds duration : durations) {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    orbit_base::Future<Timeout> timeout = CreateTimeout(duration);

    WaitForFutureToComplete(timeout);
    EXPECT_GE(std::chrono::steady_clock::now(), start + duration);
  }
}
}  // namespace orbit_qt_utils
