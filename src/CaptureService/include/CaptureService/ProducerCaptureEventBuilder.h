// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CAPTURE_EVENT_BUILDER_H_
#define CAPTURE_SERVICE_CAPTURE_EVENT_BUILDER_H_

#include <string>

#include "ApiUtils/Event.h"
#include "GrpcProtos/Constants.h"
#include "Introspection/Introspection.h"
#include "ObjectUtils/CoffFile.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/Result.h"
#include "OrbitVersion/OrbitVersion.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"
#include "capture.pb.h"

namespace orbit_capture_service {

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateCaptureStartedEvent(
    const orbit_grpc_protos::CaptureOptions& capture_options, absl::Time capture_start_time,
    uint64_t capture_start_timestamp_ns);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateClockResolutionEvent(
    uint64_t timestamp_ns, uint64_t resolution_ns);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateErrorEnablingOrbitApiEvent(
    uint64_t timestamp_ns, std::string message);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent
CreateErrorEnablingUserSpaceInstrumentationEvent(uint64_t timestamp_ns, std::string message);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateWarningEvent(uint64_t timestamp_ns,
                                                                         std::string message);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateCaptureFinishedEvent();

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CAPTURE_EVENT_BUILDER_H_
