// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracing/Tracer.h"

#include "OrbitBase/ThreadUtils.h"
#include "TracerThread.h"

namespace orbit_linux_tracing {

void Tracer::Run() {
  orbit_base::SetCurrentThreadName("Tracer::Run");
  TracerThread session{capture_options_};
  session.SetListener(listener_);
  session.Run(exit_requested_);
}

}  // namespace orbit_linux_tracing
