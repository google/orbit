// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ApiUtils/Event.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"

static void TestScopes() {
  ORBIT_SCOPE("TEST_ORBIT_SCOPE_1");
  ORBIT_SCOPE("TEST_ORBIT_SCOPE_2");
  ORBIT_SCOPE("TEST_ORBIT_SCOPE_3");
  ORBIT_START("TEST_ORBIT_START_4");
  ORBIT_STOP();
}

namespace orbit_introspection {

namespace {
inline int32_t RetrieveThreadId(const orbit_api::ApiScopeStart& scope_start) {
  return scope_start.meta_data.tid;
}
inline int32_t RetrieveThreadId(const orbit_api::ApiScopeStop& scope_stop) {
  return scope_stop.meta_data.tid;
}
inline int32_t RetrieveThreadId(const orbit_api::ApiScopeStartAsync& /*scope_start_async*/) {
  ORBIT_UNREACHABLE();
}
inline int32_t RetrieveThreadId(const orbit_api::ApiScopeStopAsync& /*scope_stop_async*/) {
  ORBIT_UNREACHABLE();
}
inline int32_t RetrieveThreadId(const orbit_api::ApiStringEvent& /*string_event*/) {
  ORBIT_UNREACHABLE();
}
inline int32_t RetrieveThreadId(const orbit_api::ApiTrackDouble& /*track_double*/) {
  ORBIT_UNREACHABLE();
}
inline int32_t RetrieveThreadId(const orbit_api::ApiTrackFloat& /*track_float*/) {
  ORBIT_UNREACHABLE();
}
inline int32_t RetrieveThreadId(const orbit_api::ApiTrackInt& /*track_int*/) {
  ORBIT_UNREACHABLE();
}
inline int32_t RetrieveThreadId(const orbit_api::ApiTrackInt64& /*track_int64*/) {
  ORBIT_UNREACHABLE();
}
inline int32_t RetrieveThreadId(const orbit_api::ApiTrackUint& /*track_uint*/) {
  ORBIT_UNREACHABLE();
}
inline int32_t RetrieveThreadId(const orbit_api::ApiTrackUint64& /*track_uint64*/) {
  ORBIT_UNREACHABLE();
}

// The variant type `ApiEventVariant` requires to contain `std::monostate` in order to be default-
// constructable. However, that state is never expected to be called in the visitor.
inline int32_t RetrieveThreadId(const std::monostate& /*unused*/) { ORBIT_UNREACHABLE(); }
}  // namespace

TEST(Tracing, Scopes) {
  constexpr size_t kNumThreads = 10;
  constexpr size_t kNumExpectedScopesPerThread = 8;

  absl::flat_hash_map<uint32_t, std::vector<orbit_api::ApiEventVariant>> scopes_by_thread_id;
  std::once_flag thread_id_set_flag;
  uint32_t callback_thread_id = 0;
  {
    IntrospectionListener tracing_listener(
        [&scopes_by_thread_id, &thread_id_set_flag,
         &callback_thread_id](const orbit_api::ApiEventVariant& api_event) {
          // Check that callback is called from a single thread.
          std::call_once(thread_id_set_flag, [&callback_thread_id] {
            callback_thread_id = orbit_base::GetCurrentThreadId();
          });
          EXPECT_EQ(orbit_base::GetCurrentThreadId(), callback_thread_id);
          std::visit(
              [&scopes_by_thread_id](const auto& event) {
                int32_t tid = RetrieveThreadId(event);
                scopes_by_thread_id[tid].emplace_back(event);
              },
              api_event);
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

}  // namespace orbit_introspection
