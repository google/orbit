// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsTracing/Tracer.h"

#include "TracerImpl.h"

namespace orbit_windows_tracing {

std::unique_ptr<Tracer> Tracer::Create(orbit_grpc_protos::CaptureOptions capture_options,
                                       TracerListener* listener) {
  return std::make_unique<TracerImpl>(capture_options, listener);
}

}  // namespace orbit_windows_tracing
