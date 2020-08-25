// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Tracing.h"

#include <memory>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"

using orbit::tracing::Scope;
using orbit::tracing::ScopeType;
using orbit::tracing::TimerCallback;

thread_local std::vector<Scope> scopes;

TimerCallback* g_callback;

namespace orbit::tracing {
Listener::Listener(const TimerCallback& callback) {
  // Note: only one listener is supported.
  absl::MutexLock lock(&mutex_);
  CHECK(g_callback == nullptr);
  callback_ = callback;
  g_callback = &callback_;
}

Listener::~Listener() {
  absl::MutexLock lock(&mutex_);
  g_callback = nullptr;
}

}  // namespace orbit::tracing

namespace orbit_api {

void Start(const char* name, uint64_t, orbit::Color color) {
  scopes.emplace_back(orbit::tracing::Scope());
  auto& scope = scopes.back();
  scope.name = name;
  scope.begin = MonotonicTimestampNs();
  scope.color = color;
}

void Stop() {
  auto& scope = scopes.back();
  scope.end = MonotonicTimestampNs();
  scope.depth = scopes.size() - 1;
  scope.tid = GetCurrentThreadId();
  scope.type = ScopeType::kScope;

  {
    absl::MutexLock lock(orbit::tracing::Listener::GetMutex());
    if (g_callback != nullptr) (*g_callback)(std::move(scope));
  }

  scopes.pop_back();
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

  absl::MutexLock lock(orbit::tracing::Listener::GetMutex());
  if (g_callback != nullptr) (*g_callback)(std::move(scope));
}

template <typename Dest, typename Source>
inline Dest encode(const Source& source) {
  static_assert(sizeof(Source) <= sizeof(Dest));
  Dest dest = 0;
  std::memcpy(&dest, &source, sizeof(Source));
  return dest;
}

void TrackInt(const char* name, int32_t value, orbit::Color color) {
  TrackScope(name, encode<uint64_t>(value), color, ScopeType::kTrackInt);
}

void TrackInt64(const char* name, int64_t value, orbit::Color color) {
  TrackScope(name, encode<uint64_t>(value), color, ScopeType::kTrackInt64);
}

void TrackUint(const char* name, uint32_t value, orbit::Color color) {
  TrackScope(name, encode<uint64_t>(value), color, ScopeType::kTrackUint);
}

void TrackUint64(const char* name, uint64_t value, orbit::Color color) {
  TrackScope(name, value, color, ScopeType::kTrackUint64);
}

void TrackFloat(const char* name, float value, orbit::Color color) {
  TrackScope(name, encode<uint64_t>(value), color, ScopeType::kTrackFloatAsInt);
}

void TrackDouble(const char* name, double value, orbit::Color color) {
  TrackScope(name, encode<uint64_t>(value), color, ScopeType::kTrackDoubleAsInt64);
}

}  // namespace orbit_api
