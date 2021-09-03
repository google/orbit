// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_TRACER_LISTENER_H_
#define WINDOWS_TRACING_TRACER_LISTENER_H_

#include "capture.pb.h"

namespace orbit_windows_tracing {

class TracerListener {
 public:
  virtual ~TracerListener() = default;
  virtual void OnSchedulingSlice(orbit_grpc_protos::SchedulingSlice scheduling_slice) = 0;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_TRACER_LISTENER_H_
