// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_MOCK_TRACER_LISTENER_H_
#define LINUX_TRACING_MOCK_TRACER_LISTENER_H_

#include "LinuxTracing/TracerListener.h"

namespace orbit_linux_tracing {

class MockTracerListener : public TracerListener {
 public:
  MOCK_METHOD(void, OnSchedulingSlice, (orbit_grpc_protos::SchedulingSlice), (override));
  MOCK_METHOD(void, OnCallstackSample, (orbit_grpc_protos::FullCallstackSample), (override));
  MOCK_METHOD(void, OnFunctionCall, (orbit_grpc_protos::FunctionCall), (override));
  MOCK_METHOD(void, OnThreadStateSliceCallstack, (orbit_grpc_protos::ThreadStateSliceCallstack),
              (override));
  MOCK_METHOD(void, OnGpuJob, (orbit_grpc_protos::FullGpuJob full_gpu_job), (override));
  MOCK_METHOD(void, OnThreadName, (orbit_grpc_protos::ThreadName), (override));
  MOCK_METHOD(void, OnThreadNamesSnapshot, (orbit_grpc_protos::ThreadNamesSnapshot), (override));
  MOCK_METHOD(void, OnThreadStateSlice, (orbit_grpc_protos::ThreadStateSlice), (override));
  MOCK_METHOD(void, OnAddressInfo, (orbit_grpc_protos::FullAddressInfo), (override));
  MOCK_METHOD(void, OnTracepointEvent, (orbit_grpc_protos::FullTracepointEvent), (override));
  MOCK_METHOD(void, OnModuleUpdate, (orbit_grpc_protos::ModuleUpdateEvent), (override));
  MOCK_METHOD(void, OnModulesSnapshot, (orbit_grpc_protos::ModulesSnapshot), (override));
  MOCK_METHOD(void, OnErrorsWithPerfEventOpenEvent,
              (orbit_grpc_protos::ErrorsWithPerfEventOpenEvent), (override));
  MOCK_METHOD(void, OnLostPerfRecordsEvent, (orbit_grpc_protos::LostPerfRecordsEvent), (override));
  MOCK_METHOD(void, OnOutOfOrderEventsDiscardedEvent,
              (orbit_grpc_protos::OutOfOrderEventsDiscardedEvent), (override));
  MOCK_METHOD(void, OnWarningInstrumentingWithUprobesEvent,
              (orbit_grpc_protos::WarningInstrumentingWithUprobesEvent), (override));
  MOCK_METHOD(void, OnApiScopeStart, (orbit_grpc_protos::ApiScopeStart), (override));
  MOCK_METHOD(void, OnApiScopeStartAsync, (orbit_grpc_protos::ApiScopeStartAsync), (override));
  MOCK_METHOD(void, OnApiScopeStop, (orbit_grpc_protos::ApiScopeStop), (override));
  MOCK_METHOD(void, OnApiScopeStopAsync, (orbit_grpc_protos::ApiScopeStopAsync), (override));
  MOCK_METHOD(void, OnApiStringEvent, (orbit_grpc_protos::ApiStringEvent api_string_event),
              (override));
  MOCK_METHOD(void, OnApiTrackDouble, (orbit_grpc_protos::ApiTrackDouble api_track_double),
              (override));
  MOCK_METHOD(void, OnApiTrackFloat, (orbit_grpc_protos::ApiTrackFloat api_track_float),
              (override));
  MOCK_METHOD(void, OnApiTrackInt, (orbit_grpc_protos::ApiTrackInt api_track_int), (override));
  MOCK_METHOD(void, OnApiTrackInt64, (orbit_grpc_protos::ApiTrackInt64 api_track_int64),
              (override));
  MOCK_METHOD(void, OnApiTrackUint, (orbit_grpc_protos::ApiTrackUint api_track_uint), (override));
  MOCK_METHOD(void, OnApiTrackUint64, (orbit_grpc_protos::ApiTrackUint64 api_track_uint64),
              (override));
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_MOCK_TRACER_LISTENER_H_
