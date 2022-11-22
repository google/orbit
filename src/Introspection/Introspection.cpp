// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Introspection/Introspection.h"

#include <absl/base/attributes.h>
#include <absl/base/const_init.h>
#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <stdint.h>

#include <atomic>
#include <cstring>
#include <memory>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitBase/ThreadUtils.h"

using orbit_api::ApiEventVariant;
using orbit_introspection::IntrospectionEventCallback;
using orbit_introspection::IntrospectionListener;

ABSL_CONST_INIT static absl::Mutex global_introspection_mutex(absl::kConstInit);
ABSL_CONST_INIT static IntrospectionListener* global_introspection_listener
    ABSL_GUARDED_BY(global_introspection_mutex) = nullptr;

// Introspection uses the same function table used by the Orbit API, but specifies its own
// functions.
orbit_api_v2 g_orbit_api;

namespace orbit_introspection {

void InitializeIntrospection();

IntrospectionListener::IntrospectionListener(IntrospectionEventCallback callback)
    : user_callback_{std::move(callback)} {
  constexpr size_t kMinNumThreads = 1;
  constexpr size_t kMaxNumThreads = 1;
  thread_pool_ =
      orbit_base::ThreadPool::Create(kMinNumThreads, kMaxNumThreads, absl::Milliseconds(500));

  // Activate listener (only one listener instance is supported).
  absl::MutexLock lock(&global_introspection_mutex);
  ORBIT_CHECK(!IsActive());
  InitializeIntrospection();
  global_introspection_listener = this;
  active_ = true;
  shutdown_initiated_ = false;
}

IntrospectionListener::~IntrospectionListener() {
  // Communicate that the thread pool will be shut down before shutting down
  // the thread pool itself.
  // Note that this is required, as otherwise, we might allow scheduling
  // new events on the already shut down thread pool.
  {
    absl::MutexLock lock(&global_introspection_mutex);
    ORBIT_CHECK(IsActive());
    shutdown_initiated_ = true;
  }
  // Purge deferred scopes.
  thread_pool_->Shutdown();
  thread_pool_->Wait();

  // Deactivate and destroy the listener.
  absl::MutexLock lock(&global_introspection_mutex);
  active_ = false;
  global_introspection_listener = nullptr;
}

}  // namespace orbit_introspection

namespace {
struct ScopeToggle {
  ScopeToggle() = delete;
  ScopeToggle(bool* value, bool initial_value) : toggle(value) { *toggle = initial_value; }
  ~ScopeToggle() { *toggle = !*toggle; }
  bool* toggle = nullptr;
};
}  // namespace

void IntrospectionListener::DeferApiEventProcessing(const orbit_api::ApiEventVariant& api_event) {
  // Prevent reentry to avoid feedback loop.
  thread_local bool is_internal_update = false;
  if (is_internal_update) return;

  // User callback is called from a worker thread to minimize contention on instrumented threads.
  absl::MutexLock lock(&global_introspection_mutex);
  if (IsShutdownInitiated()) return;
  global_introspection_listener->thread_pool_->Schedule([api_event]() {
    ScopeToggle scope_toggle(&is_internal_update, true);
    absl::MutexLock lock(&global_introspection_mutex);
    if (!IsActive()) return;
    global_introspection_listener->user_callback_(api_event);
  });
}

void orbit_api_start_v1(const char* name, orbit_api_color color, uint64_t group_id,
                        uint64_t caller_address) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  if (caller_address == kOrbitCallerAddressAuto) {
    caller_address = ORBIT_GET_CALLER_PC();
  }
  orbit_api::ApiScopeStart api_scope_start{process_id, thread_id, timestamp_ns,  name,
                                           color,      group_id,  caller_address};
  IntrospectionListener::DeferApiEventProcessing(api_scope_start);
}

void orbit_api_stop() {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiScopeStop api_scope_stop{process_id, thread_id, timestamp_ns};
  IntrospectionListener::DeferApiEventProcessing(api_scope_stop);
}

void orbit_api_start_async_v1(const char* name, uint64_t id, orbit_api_color color,
                              uint64_t caller_address) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  if (caller_address == kOrbitCallerAddressAuto) {
    caller_address = ORBIT_GET_CALLER_PC();
  }
  orbit_api::ApiScopeStartAsync api_scope_start_async{process_id, thread_id, timestamp_ns,  name,
                                                      id,         color,     caller_address};
  IntrospectionListener::DeferApiEventProcessing(api_scope_start_async);
}

void orbit_api_stop_async(uint64_t id) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiScopeStopAsync api_scope_stop_async{process_id, thread_id, timestamp_ns, id};
  IntrospectionListener::DeferApiEventProcessing(api_scope_stop_async);
}

void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiStringEvent api_string_event{process_id, thread_id, timestamp_ns, str, id, color};
  IntrospectionListener::DeferApiEventProcessing(api_string_event);
}

void orbit_api_track_int(const char* name, int value, orbit_api_color color) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackInt api_track{process_id, thread_id, timestamp_ns, name, value, color};
  IntrospectionListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_int64(const char* name, int64_t value, orbit_api_color color) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackInt64 api_track{process_id, thread_id, timestamp_ns, name, value, color};
  IntrospectionListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_uint(const char* name, uint32_t value, orbit_api_color color) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackUint api_track{process_id, thread_id, timestamp_ns, name, value, color};
  IntrospectionListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_uint64(const char* name, uint64_t value, orbit_api_color color) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackUint64 api_track{process_id, thread_id, timestamp_ns, name, value, color};
  IntrospectionListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_float(const char* name, float value, orbit_api_color color) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackFloat api_track{process_id, thread_id, timestamp_ns, name, value, color};
  IntrospectionListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_double(const char* name, double value, orbit_api_color color) {
  uint32_t process_id = orbit_base::GetCurrentProcessId();
  uint32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackDouble api_track{process_id, thread_id, timestamp_ns, name, value, color};
  IntrospectionListener::DeferApiEventProcessing(api_track);
}

namespace orbit_introspection {

void InitializeIntrospection() {
  if (g_orbit_api.initialized != 0) return;
  g_orbit_api.start = &orbit_api_start_v1;
  g_orbit_api.stop = &orbit_api_stop;
  g_orbit_api.start_async = &orbit_api_start_async_v1;
  g_orbit_api.stop_async = &orbit_api_stop_async;
  g_orbit_api.async_string = &orbit_api_async_string;
  g_orbit_api.track_int = &orbit_api_track_int;
  g_orbit_api.track_int64 = &orbit_api_track_int64;
  g_orbit_api.track_uint = &orbit_api_track_uint;
  g_orbit_api.track_uint64 = &orbit_api_track_uint64;
  g_orbit_api.track_float = &orbit_api_track_float;
  g_orbit_api.track_double = &orbit_api_track_double;
  std::atomic_thread_fence(std::memory_order_release);
  g_orbit_api.initialized = 1;
  g_orbit_api.enabled = 1;
}

}  // namespace orbit_introspection
