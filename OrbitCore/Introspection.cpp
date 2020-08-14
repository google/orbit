// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Introspection.h"

#if ORBIT_TRACING_ENABLED

#include <memory>
#include <vector>

#include "OrbitBase/Tracing.h"
#include "Utils.h"

#ifdef _WIN32
namespace orbit::tracing {

// Instantiate tracing handler. On Linux, see OrbitTracing.cpp.
std::unique_ptr<Handler> GHandler;

}  // namespace orbit::tracing
#endif

namespace orbit::introspection {

thread_local std::vector<Scope> scopes;

Handler::Handler(LinuxTracingBuffer* tracing_buffer) : tracing_buffer_(tracing_buffer) {}

void Handler::Begin(const char* name) {
  scopes.emplace_back(Scope{Timer(), name});
  scopes.back().timer_.Start();
}

void Handler::End() {
  Scope& scope = scopes.back();

  scope.timer_.Stop();
  scope.timer_.m_Type = Timer::INTROSPECTION;
  scope.timer_.m_Depth = scopes.size() - 1;

  uint64_t hash = StringHash(scope.name_);
  tracing_buffer_->RecordKeyAndString(hash, scope.name_);
  scope.timer_.m_UserData[0] = hash;

  tracing_buffer_->RecordTimer(std::move(scope.timer_));

  scopes.pop_back();
}

void Handler::Track(const char*, int) {}

void Handler::Track(const char*, float) {}

}  // namespace orbit::introspection

#endif  // ORBIT_TRACING_ENABLED
