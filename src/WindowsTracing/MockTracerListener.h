// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_MOCK_TRACER_LISTENER_H_
#define WINDOWS_TRACING_MOCK_TRACER_LISTENER_H_

#include <vector>

#include "WindowsTracing/TracerListener.h"
#include "capture.pb.h"
namespace orbit_windows_tracing {
class MockTracerListener : public orbit_windows_tracing::TracerListener {
 public:
  MOCK_METHOD(void, OnSchedulingSlice, (orbit_grpc_protos::SchedulingSlice), (override));
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_MOCK_TRACER_LISTENER_H_
