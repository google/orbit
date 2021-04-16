// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>

#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "OrbitBase/ThreadUtils.h"
#include "OrbitBase/Tracing.h"
#include "absl/container/flat_hash_map.h"

using orbit_base::TracingListener;
using orbit_base::TracingScope;
using orbit_base::TracingTimerCallback;

void TestScopes() {
  ORBIT_SCOPE("TEST_ORBIT_SCOPE_1");
  ORBIT_SCOPE("TEST_ORBIT_SCOPE_2");
  ORBIT_SCOPE("TEST_ORBIT_SCOPE_3");
  ORBIT_START("TEST_ORBIT_START_4");
  ORBIT_STOP();
}

TEST(Tracing, Scopes) {
  constexpr size_t kNumThreads = 10;
  constexpr size_t kNumExpectedScopesPerThread = 4;

  absl::flat_hash_map<uint32_t, std::vector<TracingScope>> scopes_by_thread_id;
  std::once_flag thread_id_set_flag;
  uint32_t callback_thread_id = 0;
  {
    TracingListener tracing_listener([&scopes_by_thread_id, &thread_id_set_flag,
                                      &callback_thread_id](const TracingScope& scope) {
      // Check that callback is called from a single thread.
      std::call_once(thread_id_set_flag, [&callback_thread_id] {
        callback_thread_id = orbit_base::GetCurrentThreadId();
      });
      EXPECT_EQ(orbit_base::GetCurrentThreadId(), callback_thread_id);
      scopes_by_thread_id[scope.tid].emplace_back(scope);
    });

    std::vector<std::unique_ptr<std::thread>> threads;
    for (size_t i = 0; i < kNumThreads; ++i) {
      threads.emplace_back(std::make_unique<std::thread>([] { TestScopes(); }));
    }

    for (auto& thread : threads) {
      thread->join();
    }
  }

  EXPECT_EQ(scopes_by_thread_id.size(), kNumThreads);
  for (const auto& pair : scopes_by_thread_id) {
    EXPECT_EQ(pair.second.size(), kNumExpectedScopesPerThread);
  }
}
