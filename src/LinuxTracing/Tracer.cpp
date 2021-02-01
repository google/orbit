// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracing/Tracer.h"

#include <pthread.h>

#include <atomic>
#include <memory>

#include "LinuxTracing/TracerListener.h"
#include "TracerThread.h"
#include "capture.pb.h"

namespace orbit_linux_tracing {

using orbit_grpc_protos::CaptureOptions;

void Tracer::Run() {
  pthread_setname_np(pthread_self(), "Tracer::Run");
  TracerThread session{capture_options_};
  session.SetListener(listener_);
  session.Run(exit_requested_);
}

}  // namespace orbit_linux_tracing
