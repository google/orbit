// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Introspection/Introspection.h"

#include <absl/base/attributes.h>
#include <absl/base/const_init.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <string.h>

#include <memory>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitBase/ThreadUtils.h"

using orbit_introspection::TracingListener;
using orbit_introspection::TracingScope;
using orbit_introspection::TracingTimerCallback;

ABSL_CONST_INIT static absl::Mutex global_tracing_mutex(absl::kConstInit);
ABSL_CONST_INIT static TracingListener* global_tracing_listener = nullptr;

// Tracing uses the same function table used by the Orbit API, but specifies its own functions.
orbit_api_v0 g_orbit_api_v0;

namespace orbit_introspection {

void InitializeTracing();

TracingScope::TracingScope(orbit_api::EventType type, const char* name, uint64_t data,
                           orbit_api_color color)
    : encoded_event(type, name, data, color) {}

TracingListener::TracingListener(TracingTimerCallback callback) {
  constexpr size_t kMinNumThreads = 1;
  constexpr size_t kMaxNumThreads = 1;
  thread_pool_ = ThreadPool::Create(kMinNumThreads, kMaxNumThreads, absl::Milliseconds(500));
  user_callback_ = std::move(callback);

  // Activate listener (only one listener instance is supported).
  absl::MutexLock lock(&global_tracing_mutex);
  CHECK(!IsActive());
  InitializeTracing();
  global_tracing_listener = this;
  active_ = true;
  shutdown_initiated_ = false;
}

TracingListener::~TracingListener() {
  // Communicate that the thread pool will be shut down before shutting down
  // the thread pool itself.
  // Note that this is required, as otherwise, we might allow scheduling
  // new events on the already shut down thread pool.
  {
    absl::MutexLock lock(&global_tracing_mutex);
    CHECK(IsActive());
    shutdown_initiated_ = true;
  }
  // Purge deferred scopes.
  thread_pool_->Shutdown();
  thread_pool_->Wait();

  // Deactivate and destroy the listener.
  absl::MutexLock lock(&global_tracing_mutex);
  active_ = false;
  global_tracing_listener = nullptr;
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

void TracingListener::DeferScopeProcessing(const TracingScope& scope) {
  // Prevent reentry to avoid feedback loop.
  thread_local bool is_internal_update = false;
  if (is_internal_update) return;

  // User callback is called from a worker thread to minimize contention on instrumented threads.
  absl::MutexLock lock(&global_tracing_mutex);
  if (IsShutdownInitiated()) return;
  global_tracing_listener->thread_pool_->Schedule([scope]() {
    ScopeToggle scope_toggle(&is_internal_update, true);
    absl::MutexLock lock(&global_tracing_mutex);
    if (!IsActive()) return;
    global_tracing_listener->user_callback_(scope);
  });
}

static std::vector<TracingScope>& GetThreadLocalScopes() {
  thread_local std::vector<TracingScope> thread_local_scopes;
  return thread_local_scopes;
}

void orbit_api_start(const char* name, orbit_api_color color) {
  GetThreadLocalScopes().emplace_back(
      TracingScope(orbit_api::kScopeStart, name, /*data*/ 0, color));
  auto& scope = GetThreadLocalScopes().back();
  scope.begin = orbit_base::CaptureTimestampNs();
}

void orbit_api_stop() {
  std::vector<TracingScope>& thread_local_scopes = GetThreadLocalScopes();
  if (thread_local_scopes.size() == 0) return;
  auto& scope = thread_local_scopes.back();
  scope.end = orbit_base::CaptureTimestampNs();
  scope.depth = GetThreadLocalScopes().size() - 1;
  scope.tid = static_cast<uint32_t>(orbit_base::GetCurrentThreadId());
  TracingListener::DeferScopeProcessing(scope);
  GetThreadLocalScopes().pop_back();
}

void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  TracingScope scope(orbit_api::kScopeStartAsync, name, id, color);
  scope.begin = orbit_base::CaptureTimestampNs();
  scope.end = scope.begin;
  scope.tid = static_cast<uint32_t>(orbit_base::GetCurrentThreadId());
  TracingListener::DeferScopeProcessing(scope);
}

void orbit_api_stop_async(uint64_t id) {
  TracingScope scope(orbit_api::kScopeStopAsync, /*name*/ nullptr, id);
  scope.begin = orbit_base::CaptureTimestampNs();
  scope.end = scope.begin;
  scope.tid = static_cast<uint32_t>(orbit_base::GetCurrentThreadId());
  TracingListener::DeferScopeProcessing(scope);
}

void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color) {
  if (str == nullptr) return;
  TracingScope scope(orbit_api::kString, /*name*/ nullptr, id, color);
  auto& e = scope.encoded_event;
  constexpr size_t chunk_size = orbit_api::kMaxEventStringSize - 1;
  const char* end = str + strlen(str);
  while (str < end) {
    std::strncpy(e.event.name, str, chunk_size);
    e.event.name[chunk_size] = 0;
    TracingListener::DeferScopeProcessing(scope);
    str += chunk_size;
  }
}

static inline void TrackValue(orbit_api::EventType type, const char* name, uint64_t value,
                              orbit_api_color color) {
  TracingScope scope(type, name, value, color);
  scope.begin = orbit_base::CaptureTimestampNs();
  scope.tid = static_cast<uint32_t>(orbit_base::GetCurrentThreadId());
  TracingListener::DeferScopeProcessing(scope);
}

void orbit_api_track_int(const char* name, int value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackInt, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_int64(const char* name, int64_t value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackInt64, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_uint(const char* name, uint32_t value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackUint, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_uint64(const char* name, uint64_t value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackUint64, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_float(const char* name, float value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackFloat, name, orbit_api::Encode<uint64_t>(value), color);
}

void orbit_api_track_double(const char* name, double value, orbit_api_color color) {
  TrackValue(orbit_api::kTrackDouble, name, orbit_api::Encode<uint64_t>(value), color);
}

namespace orbit_introspection {

void InitializeTracing() {
  if (g_orbit_api_v0.initialized != 0) return;
  g_orbit_api_v0.start = &orbit_api_start;
  g_orbit_api_v0.stop = &orbit_api_stop;
  g_orbit_api_v0.start_async = &orbit_api_start_async;
  g_orbit_api_v0.stop_async = &orbit_api_stop_async;
  g_orbit_api_v0.async_string = &orbit_api_async_string;
  g_orbit_api_v0.track_int = &orbit_api_track_int;
  g_orbit_api_v0.track_int64 = &orbit_api_track_int64;
  g_orbit_api_v0.track_uint = &orbit_api_track_uint;
  g_orbit_api_v0.track_uint64 = &orbit_api_track_uint64;
  g_orbit_api_v0.track_float = &orbit_api_track_float;
  g_orbit_api_v0.track_double = &orbit_api_track_double;
  std::atomic_thread_fence(std::memory_order_release);
  g_orbit_api_v0.initialized = 1;
  g_orbit_api_v0.enabled = 1;
}

}  // namespace orbit_introspection
