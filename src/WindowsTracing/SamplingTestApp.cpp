// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Logging.h"
#include "WindowsTracing/Tracer.h"
#include "capture.pb.h"

namespace orbit_windows_tracing {

class DummyTracerListener : public TracerListener {
 public:
  virtual void OnSchedulingSlice(orbit_grpc_protos::SchedulingSlice /*scheduling_slice*/){};
  virtual void OnCallstackSample(orbit_grpc_protos::FullCallstackSample /*callstack_sample*/){};
};

void Run() {
  orbit_grpc_protos::CaptureOptions capture_options;
  capture_options.set_samples_per_second(8000.0);
  DummyTracerListener listener;
  std::unique_ptr<Tracer> tracer = Tracer::Create(capture_options, &listener);
  tracer->Start();
  static constexpr uint32_t kTestDurationInSeconds = 10;
  std::this_thread::sleep_for(std::chrono::seconds(kTestDurationInSeconds));
  tracer->Stop();
}

}  // namespace orbit_windows_tracing

int main() { orbit_windows_tracing::Run(); }