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

Scope::Scope(orbit_api::EventType type, const char* name, uint64_t data, orbit::Color color)
    : encoded_event(type, name, data, color) {}

Listener::Listener(TimerCallback callback) {
  constexpr size_t kMinNumThreads = 1;
  constexpr size_t kMaxNumThreads = 1;
  thread_pool_ = ThreadPool::Create(kMinNumThreads, kMaxNumThreads, absl::Milliseconds(500));
  thread_pool_->EnableAutoProfiling(false);  // To prevent feedback loop.
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
    global_tracing_listener->user_callback_(scope);
  });
}

#ifdef ORBIT_API_INTERNAL_IMPL

static std::vector<Scope>& GetThreadLocalScopes() {
  thread_local std::vector<Scope> thread_local_scopes;
  return thread_local_scopes;
}

namespace orbit_api {

void Start(const char* name, orbit::Color color) {
  GetThreadLocalScopes().emplace_back(
      orbit::tracing::Scope(orbit_api::kScopeStart, name, /*data*/ 0, color));
  auto& scope = GetThreadLocalScopes().back();
  scope.begin = MonotonicTimestampNs();
}

void Stop() {
  auto& scope = GetThreadLocalScopes().back();
  scope.end = MonotonicTimestampNs();
  scope.depth = GetThreadLocalScopes().size() - 1;
  scope.tid = GetCurrentThreadId();
  Listener::DeferScopeProcessing(scope);
  GetThreadLocalScopes().pop_back();
}

void StartAsync(const char* name, uint64_t id, orbit::Color color) {
  orbit::tracing::Scope scope(orbit_api::kScopeStartAsync, name, id, color);
  scope.begin = MonotonicTimestampNs();
  scope.end = scope.begin;
  scope.tid = GetCurrentThreadId();
  Listener::DeferScopeProcessing(scope);
}

void StopAsync(uint64_t id) {
  orbit::tracing::Scope scope(orbit_api::kScopeStopAsync, /*name*/ nullptr, id);
  scope.begin = MonotonicTimestampNs();
  scope.end = scope.begin;
  scope.tid = GetCurrentThreadId();
  Listener::DeferScopeProcessing(scope);
}

void AsyncString(const char* str, uint64_t id, orbit::Color color) {
  if (str == nullptr) return;
  orbit::tracing::Scope scope(orbit_api::kString, /*name*/ nullptr, id, color);
  auto& e = scope.encoded_event;
  constexpr size_t chunk_size = kMaxEventStringSize - 1;
  const char* end = str + strlen(str);
  while (str < end) {
    std::strncpy(e.event.name, str, chunk_size);
    e.event.name[chunk_size] = 0;
    Listener::DeferScopeProcessing(scope);
    str += chunk_size;
  }
}

void TrackValue(orbit_api::EventType type, const char* name, uint64_t value, orbit::Color color) {
  orbit::tracing::Scope scope(type, name, value, color);
  scope.begin = MonotonicTimestampNs();
  scope.tid = GetCurrentThreadId();
  Listener::DeferScopeProcessing(scope);
}

}  // namespace orbit_api

#endif
