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
using orbit::tracing::TimerCallback;

ABSL_CONST_INIT static absl::Mutex global_tracing_mutex(absl::kConstInit);
ABSL_CONST_INIT static Listener* global_tracing_listener = nullptr;

namespace orbit::tracing {

Listener::Listener(std::unique_ptr<TimerCallback> callback) {
  constexpr size_t kMinNumThreads = 1;
  constexpr size_t kMaxNumThreads = 1;
  thread_pool_ = ThreadPool::Create(kMinNumThreads, kMaxNumThreads, absl::Milliseconds(500));
  user_callback_ = std::move(callback);

  // Activate listener (only one listener instance is supported).
  absl::MutexLock lock(&global_tracing_mutex);
  CHECK(!IsActive());
  global_tracing_listener = this;
  active_ = true;
}

Listener::~Listener() {
  // Purge deferred scopes.
  thread_pool_->Shutdown();
  thread_pool_->Wait();

  // Deactivate listener.
  absl::MutexLock lock(&global_tracing_mutex);
  CHECK(IsActive());
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
    if (!IsActive()) return;
    (*global_tracing_listener->user_callback_)(scope);
  });
}

#ifdef ORBIT_API_INTERNAL_IMPL

static std::vector<Scope>& GetThreadLocalScopes() {
  thread_local std::vector<Scope> thread_local_scopes;
  return thread_local_scopes;
}

namespace orbit_api {

void Start(const char* name, orbit::Color color) {
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
  scope.type = orbit_api::EventType::kScopeStop;
  Listener::DeferScopeProcessing(scope);
  GetThreadLocalScopes().pop_back();
}

// Will be implemented shortly.
void StartAsync(const char*, uint64_t, orbit::Color) { CHECK(0); }
void StopAsync(uint64_t) { CHECK(0); }

void TrackScope(const char* name, uint64_t value, orbit::Color color, orbit_api::EventType type) {
  orbit::tracing::Scope scope;
  scope.begin = MonotonicTimestampNs();
  scope.name = name;
  scope.color = color;
  scope.tracked_value = value;
  scope.tid = GetCurrentThreadId();
  scope.type = type;
  Listener::DeferScopeProcessing(scope);
}

void TrackValue(orbit_api::EventType type, const char* name, uint64_t value, orbit::Color color) {
  TrackScope(name, value, color, type);
}

}  // namespace orbit_api

#endif
