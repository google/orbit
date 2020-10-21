// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include "OrbitBase/Profiling.h"
#include "OrbitBase/Tracing.h"
#include "absl/container/flat_hash_map.h"

using orbit::tracing::Listener;
using orbit::tracing::Scope;
using orbit::tracing::TimerCallback;

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

  absl::flat_hash_map<pid_t, std::vector<Scope>> scopes_by_thread_id;
  {
    Listener tracing_listener([&scopes_by_thread_id](const Scope& scope) {
      // Check that callback is called from a single thread.
      static pid_t callback_thread_id = GetCurrentThreadId();
      EXPECT_EQ(GetCurrentThreadId(), callback_thread_id);
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
