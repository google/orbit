// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WINDOWS_TRACING_TRACER_LISTENER_H_
#define WINDOWS_TRACING_TRACER_LISTENER_H_

#include "GrpcProtos/capture.pb.h"

namespace orbit_windows_tracing {

// Listener interface for receiving events from a Tracer.
class TracerListener {
 public:
  virtual ~TracerListener() = default;
  virtual void OnSchedulingSlice(orbit_grpc_protos::SchedulingSlice scheduling_slice) = 0;
  virtual void OnCallstackSample(orbit_grpc_protos::FullCallstackSample callstack_sample) = 0;
  virtual void OnFunctionCall(orbit_grpc_protos::FunctionCall function_call) = 0;
  virtual void OnModuleUpdate(orbit_grpc_protos::ModuleUpdateEvent module_update_event) = 0;
  virtual void OnModulesSnapshot(orbit_grpc_protos::ModulesSnapshot modules_snapshot) = 0;
  virtual void OnThreadNamesSnapshot(
      orbit_grpc_protos::ThreadNamesSnapshot thread_names_snapshot) = 0;
  virtual void OnPresentEvent(orbit_grpc_protos::PresentEvent present_event) = 0;
};

}  // namespace orbit_windows_tracing

#endif  // WINDOWS_TRACING_TRACER_LISTENER_H_
