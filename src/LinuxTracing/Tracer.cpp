// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracing/Tracer.h"

#include <memory>
#include <utility>

#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "TracerImpl.h"

namespace orbit_linux_tracing {

std::unique_ptr<Tracer> Tracer::Create(
    const orbit_grpc_protos::CaptureOptions& capture_options,
    std::unique_ptr<UserSpaceInstrumentationAddresses> user_space_instrumentation_addresses,
    TracerListener* listener) {
  return std::make_unique<TracerImpl>(capture_options,
                                      std::move(user_space_instrumentation_addresses), listener);
}

}  // namespace orbit_linux_tracing
