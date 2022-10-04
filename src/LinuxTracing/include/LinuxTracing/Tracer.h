// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_TRACER_H_
#define LINUX_TRACING_TRACER_H_

#include <atomic>
#include <memory>
#include <thread>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "LinuxTracing/TracerListener.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"

namespace orbit_linux_tracing {

class Tracer {
 public:
  virtual void Start() = 0;
  virtual void Stop() = 0;

  // The `Process*` methods are used to pipe back events from external producers into
  // `LinuxTracing`.
  // This way they can be processed together with other data from the sampling. The user space
  // instrumentation events have to be processed together with stack samples, allowing us to
  // correctly unwind the samples that fall inside one or more dynamically instrumented functions,
  // just like we do with u(ret)probes.
  // Besides that all the events from external producers need to have their pids/tids adjusted in
  // case the target process is running in a container.
  virtual void ProcessApiScopeStart(const orbit_grpc_protos::ApiScopeStart& api_scope_start) = 0;
  virtual void ProcessApiScopeStartAsync(
      const orbit_grpc_protos::ApiScopeStartAsync& api_scope_start_async) = 0;
  virtual void ProcessApiScopeStop(const orbit_grpc_protos::ApiScopeStop& api_scope_stop) = 0;
  virtual void ProcessApiScopeStopAsync(
      const orbit_grpc_protos::ApiScopeStopAsync& api_scope_stop_async) = 0;
  virtual void ProcessApiStringEvent(const orbit_grpc_protos::ApiStringEvent& api_string_event) = 0;
  virtual void ProcessApiTrackDouble(const orbit_grpc_protos::ApiTrackDouble& api_track_double) = 0;
  virtual void ProcessApiTrackFloat(const orbit_grpc_protos::ApiTrackFloat& api_track_float) = 0;
  virtual void ProcessApiTrackInt(const orbit_grpc_protos::ApiTrackInt& api_track_int) = 0;
  virtual void ProcessApiTrackInt64(const orbit_grpc_protos::ApiTrackInt64& api_track_int64) = 0;
  virtual void ProcessApiTrackUint(const orbit_grpc_protos::ApiTrackUint& api_track_uint) = 0;
  virtual void ProcessApiTrackUint64(const orbit_grpc_protos::ApiTrackUint64& api_track_uint64) = 0;
  virtual void ProcessFunctionEntry(const orbit_grpc_protos::FunctionEntry& function_entry) = 0;
  virtual void ProcessFunctionExit(const orbit_grpc_protos::FunctionExit& function_exit) = 0;

  virtual ~Tracer() = default;

  [[nodiscard]] static std::unique_ptr<Tracer> Create(
      const orbit_grpc_protos::CaptureOptions& capture_options,
      std::unique_ptr<UserSpaceInstrumentationAddresses> user_space_instrumentation_addresses,
      TracerListener* listener);
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_TRACER_H_
