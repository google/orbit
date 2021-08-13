// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Introspection/Introspection.h"

#include <absl/base/attributes.h>
#include <absl/base/const_init.h>
#include <absl/synchronization/mutex.h>

#include <cstring>
#include <ctime>
#include <memory>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitBase/ThreadUtils.h"

#ifdef WIN32
#include <intrin.h>

#pragma intrinsic(_ReturnAddress)
#endif

using orbit_api::ApiEventVariant;
using orbit_introspection::TracingEventCallback;
using orbit_introspection::TracingListener;

ABSL_CONST_INIT static absl::Mutex global_tracing_mutex(absl::kConstInit);
ABSL_CONST_INIT static TracingListener* global_tracing_listener = nullptr;

// Tracing uses the same function table used by the Orbit API, but specifies its own functions.
orbit_api_v1 g_orbit_api_v1;

namespace orbit_introspection {

void InitializeTracing();

TracingListener::TracingListener(TracingEventCallback callback)
    : user_callback_{std::move(callback)} {
  constexpr size_t kMinNumThreads = 1;
  constexpr size_t kMaxNumThreads = 1;
  thread_pool_ =
      orbit_base::ThreadPool::Create(kMinNumThreads, kMaxNumThreads, absl::Milliseconds(500));

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

void TracingListener::DeferApiEventProcessing(const orbit_api::ApiEventVariant& api_event) {
  // Prevent reentry to avoid feedback loop.
  thread_local bool is_internal_update = false;
  if (is_internal_update) return;

  // User callback is called from a worker thread to minimize contention on instrumented threads.
  absl::MutexLock lock(&global_tracing_mutex);
  if (IsShutdownInitiated()) return;
  global_tracing_listener->thread_pool_->Schedule([api_event]() {
    ScopeToggle scope_toggle(&is_internal_update, true);
    absl::MutexLock lock(&global_tracing_mutex);
    if (!IsActive()) return;
    global_tracing_listener->user_callback_(api_event);
  });
}

void orbit_api_start(const char* name, orbit_api_color color, uint64_t group_id) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
#ifdef WIN32
  void* return_address = _ReturnAddress();
#else
  // `__builtin_return_address(0)` will return us the (possibly encoded) return address of the
  // current function (level "0" refers to this frame, "1" would be the callers return address and
  // so on).
  // To decode the return address, we call `__builtin_extract_return_addr`.
  void* return_address = __builtin_extract_return_addr(__builtin_return_address(0));
#endif
  orbit_api::ApiScopeStart api_scope_start{process_id,
                                           thread_id,
                                           timestamp_ns,
                                           name,
                                           color,
                                           group_id,
                                           absl::bit_cast<uint64_t>(return_address)};
  TracingListener::DeferApiEventProcessing(api_scope_start);
}

void orbit_api_stop() {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiScopeStop api_scope_stop{process_id, thread_id, timestamp_ns};
  TracingListener::DeferApiEventProcessing(api_scope_stop);
}

void orbit_api_start_async(const char* name, uint64_t id, orbit_api_color color) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
#ifdef WIN32
  void* return_address = _ReturnAddress();
#else
  // `__builtin_return_address(0)` will return us the (possibly encoded) return address of the
  // current function (level "0" refers to this frame, "1" would be the callers return address and
  // so on).
  // To decode the return address, we call `__builtin_extract_return_addr`.
  void* return_address = __builtin_extract_return_addr(__builtin_return_address(0));
#endif
  orbit_api::ApiScopeStartAsync api_scope_start_async{process_id,
                                                      thread_id,
                                                      timestamp_ns,
                                                      name,
                                                      id,
                                                      color,
                                                      absl::bit_cast<uint64_t>(return_address)};

  TracingListener::DeferApiEventProcessing(api_scope_start_async);
}

void orbit_api_stop_async(uint64_t id) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiScopeStopAsync api_scope_stop_async{process_id, thread_id, timestamp_ns, id};
  TracingListener::DeferApiEventProcessing(api_scope_stop_async);
}

void orbit_api_async_string(const char* str, uint64_t id, orbit_api_color color) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiStringEvent api_string_event{process_id, thread_id, timestamp_ns, str, id, color};
  TracingListener::DeferApiEventProcessing(api_string_event);
}

void orbit_api_track_int(const char* name, int value, orbit_api_color color) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackInt api_track{process_id, thread_id, timestamp_ns, name, value, color};
  TracingListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_int64(const char* name, int64_t value, orbit_api_color color) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackInt64 api_track{process_id, thread_id, timestamp_ns, name, value, color};
  TracingListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_uint(const char* name, uint32_t value, orbit_api_color color) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackUint api_track{process_id, thread_id, timestamp_ns, name, value, color};
  TracingListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_uint64(const char* name, uint64_t value, orbit_api_color color) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackUint64 api_track{process_id, thread_id, timestamp_ns, name, value, color};
  TracingListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_float(const char* name, float value, orbit_api_color color) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackFloat api_track{process_id, thread_id, timestamp_ns, name, value, color};
  TracingListener::DeferApiEventProcessing(api_track);
}

void orbit_api_track_double(const char* name, double value, orbit_api_color color) {
  int32_t process_id = orbit_base::GetCurrentProcessId();
  int32_t thread_id = orbit_base::GetCurrentThreadId();
  uint64_t timestamp_ns = orbit_base::CaptureTimestampNs();
  orbit_api::ApiTrackDouble api_track{process_id, thread_id, timestamp_ns, name, value, color};
  TracingListener::DeferApiEventProcessing(api_track);
}

namespace orbit_introspection {

void InitializeTracing() {
  if (g_orbit_api_v1.initialized != 0) return;
  g_orbit_api_v1.start = &orbit_api_start;
  g_orbit_api_v1.stop = &orbit_api_stop;
  g_orbit_api_v1.start_async = &orbit_api_start_async;
  g_orbit_api_v1.stop_async = &orbit_api_stop_async;
  g_orbit_api_v1.async_string = &orbit_api_async_string;
  g_orbit_api_v1.track_int = &orbit_api_track_int;
  g_orbit_api_v1.track_int64 = &orbit_api_track_int64;
  g_orbit_api_v1.track_uint = &orbit_api_track_uint;
  g_orbit_api_v1.track_uint64 = &orbit_api_track_uint64;
  g_orbit_api_v1.track_float = &orbit_api_track_float;
  g_orbit_api_v1.track_double = &orbit_api_track_double;
  std::atomic_thread_fence(std::memory_order_release);
  g_orbit_api_v1.initialized = 1;
  g_orbit_api_v1.enabled = 1;
}

}  // namespace orbit_introspection
