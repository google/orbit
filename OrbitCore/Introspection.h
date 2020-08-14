// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_INTROSPECTION_H_
#define ORBIT_CORE_INTROSPECTION_H_

#include "OrbitBase/Tracing.h"
#include "ScopeTimer.h"
#include "StringManager.h"

#if ORBIT_TRACING_ENABLED

namespace orbit::introspection {

class Handler : public orbit::tracing::Handler {
 public:
  using ScopeCallback = std::function<void(const struct Scope&)>;

  explicit Handler(ScopeCallback callback);

  void Begin(const char* name) final;
  void End() final;
  void Track(const char* name, int) final;
  void Track(const char* name, float) final;

 private:
  ScopeCallback callback_;
};

struct Scope {
  Timer timer_;
  std::string name_;
};

}  // namespace orbit::introspection

#endif  // ORBIT_TRACING_ENABLED

#endif  // ORBIT_CORE_INTROSPECTION_H_
