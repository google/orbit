// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Tracing.h"

#include <memory>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadPool.h"

using orbit::tracing::Listener;
using orbit::tracing::Scope;
using orbit::tracing::ScopeType;
using orbit::tracing::TimerCallback;

ABSL_CONST_INIT static absl::Mutex global_tracing_mutex(absl::kConstInit);
ABSL_CONST_INIT static Listener* global_tracing_listener = nullptr;

static std::vector<Scope>& GetThreadLocalScopes() {
  thread_local std::vector<Scope> thread_local_scopes;
  return thread_local_scopes;
}

namespace orbit::tracing {

Listener::Listener(std::unique_ptr<TimerCallback> callback) {
  absl::MutexLock lock(&global_tracing_mutex);
  // Only one listener is supported.
  CHECK(!IsActive());
  constexpr size_t kMinNumThreads = 1;
  constexpr size_t kMaxNumThreads = 1;
  thread_pool_ = ThreadPool::Create(kMinNumThreads, kMaxNumThreads, absl::Milliseconds(500));
  user_callback_ = std::move(callback);
  global_tracing_listener = this;
  active_ = true;
}

Listener::~Listener() {
  absl::MutexLock lock(&global_tracing_mutex);
  CHECK(IsActive());
  // Purge deferred scopes.
  thread_pool_->Shutdown();
  thread_pool_->Wait();
  active_ = false;
  global_tracing_listener = nullptr;
}

}  // namespace orbit::tracing

void Listener::DeferScopeProcessing(const Scope& scope) {
  // User callback is called from a worker thread to
  // minimize contention on the instrumented threads.
  absl::MutexLock lock(&global_tracing_mutex);
  if (!IsActive()) return;
  global_tracing_listener->thread_pool_->Schedule([scope]() {
    absl::MutexLock lock(&global_tracing_mutex);
    if (IsActive() == false) return;
    (*global_tracing_listener->user_callback_)(scope);
  });
}

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
  Listener::DeferScopeProcessing(scope);
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
  Listener::DeferScopeProcessing(scope);
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
