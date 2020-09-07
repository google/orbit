// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Tracing.h"

#include <memory>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadPool.h"

using orbit::tracing::Scope;
using orbit::tracing::ScopeType;
using orbit::tracing::TimerCallback;

ABSL_CONST_INIT static absl::Mutex global_callback_mutex(absl::kConstInit);
ABSL_CONST_INIT static std::unique_ptr<TimerCallback> global_callback = {};
ABSL_CONST_INIT static absl::Mutex global_thread_pool_mutex(absl::kConstInit);
ABSL_CONST_INIT static std::unique_ptr<ThreadPool> global_thread_pool = {};

static std::vector<Scope>& GetThreadLocalScopes() {
  thread_local std::vector<Scope> thread_local_scopes;
  return thread_local_scopes;
}

static void CreateCallbackThreadPool() {
  constexpr size_t kMinNumThreads = 1;
  constexpr size_t kMaxNumThreads = 1;
  absl::MutexLock lock(&global_thread_pool_mutex);
  CHECK(global_thread_pool == nullptr);
  global_thread_pool = ThreadPool::Create(kMinNumThreads, kMaxNumThreads, absl::Milliseconds(500));
}

static void ShutdownCallbackThreadPool() {
  absl::MutexLock lock(&global_thread_pool_mutex);
  CHECK(global_thread_pool != nullptr);
  global_thread_pool->Shutdown();
  global_thread_pool->Wait();
}

static void DeferScopeProcessing(const Scope& scope) {
  // User callback is called from a worker thread to
  // minimize contention on the instrumented threads.
  global_thread_pool->Schedule([scope]() {
    absl::MutexLock lock(&global_callback_mutex);
    if (global_callback != nullptr) (*global_callback)(scope);
  });
}

namespace orbit::tracing {

Listener::Listener(std::unique_ptr<TimerCallback> callback) {
  CreateCallbackThreadPool();
  absl::MutexLock lock(&global_callback_mutex);
  // Only one listener is supported.
  CHECK(global_callback == nullptr);
  global_callback = std::move(callback);
}

Listener::~Listener() {
  {
    absl::MutexLock lock(&global_callback_mutex);
    global_callback = nullptr;
  }
  ShutdownCallbackThreadPool();
}

}  // namespace orbit::tracing

namespace orbit_api {

void Start(const char* name, uint8_t, orbit::Color color) {
  GetThreadLocalScopes().emplace_back(orbit::tracing::Scope());
  auto& scope = GetThreadLocalScopes().back();
  scope.name = name;
  scope.begin = MonotonicTimestampNs();
  scope.color = color;
}

void Stop() {
  auto& scope = GetThreadLocalScopes().back();
  scope.end = MonotonicTimestampNs();
  scope.depth = GetThreadLocalScopes().size() - 1;
  scope.tid = GetCurrentThreadId();
  scope.type = ScopeType::kScope;
  DeferScopeProcessing(scope);
  GetThreadLocalScopes().pop_back();
}

// Will be implemented shortly.
void StartAsync(const char*, uint64_t, orbit::Color) { CHECK(0); }
void StopAsync(uint64_t) { CHECK(0); }

void TrackScope(const char* name, uint64_t value, orbit::Color color, ScopeType type) {
  orbit::tracing::Scope scope;
  scope.begin = MonotonicTimestampNs();
  scope.name = name;
  scope.color = color;
  scope.tracked_value = value;
  scope.tid = GetCurrentThreadId();
  scope.type = type;
  DeferScopeProcessing(scope);
}

template <typename Dest, typename Source>
inline Dest Encode(const Source& source) {
  static_assert(sizeof(Source) <= sizeof(Dest));
  Dest dest = 0;
  std::memcpy(&dest, &source, sizeof(Source));
  return dest;
}

void TrackInt(const char* name, int32_t value, orbit::Color color) {
  TrackScope(name, Encode<uint64_t>(value), color, ScopeType::kTrackInt);
}

void TrackInt64(const char* name, int64_t value, orbit::Color color) {
  TrackScope(name, Encode<uint64_t>(value), color, ScopeType::kTrackInt64);
}

void TrackUint(const char* name, uint32_t value, orbit::Color color) {
  TrackScope(name, Encode<uint64_t>(value), color, ScopeType::kTrackUint);
}

void TrackUint64(const char* name, uint64_t value, orbit::Color color) {
  TrackScope(name, value, color, ScopeType::kTrackUint64);
}

void TrackFloat(const char* name, float value, orbit::Color color) {
  TrackScope(name, Encode<uint64_t>(value), color, ScopeType::kTrackFloatAsInt);
}

void TrackDouble(const char* name, double value, orbit::Color color) {
  TrackScope(name, Encode<uint64_t>(value), color, ScopeType::kTrackDoubleAsInt64);
}

}  // namespace orbit_api
