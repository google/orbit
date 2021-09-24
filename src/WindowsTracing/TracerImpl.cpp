// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TracerImpl.h"

#include "OrbitBase/Logging.h"

namespace orbit_windows_tracing {

TracerImpl::TracerImpl(orbit_grpc_protos::CaptureOptions capture_options, TracerListener* listener)
    : capture_options_(std::move(capture_options)), listener_(listener) {}

void TracerImpl::Start() {
  CHECK(krabs_tracer_ == nullptr);
  krabs_tracer_ = std::make_unique<KrabsTracer>(capture_options_, listener_);
  krabs_tracer_->Start();
}

void TracerImpl::Stop() {
  CHECK(krabs_tracer_ != nullptr);
  krabs_tracer_->Stop();
  krabs_tracer_ = nullptr;
}

}  // namespace orbit_windows_tracing
