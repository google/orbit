// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_COMMON_PRODUCER_CAPTURE_EVENT_BUILDERS_H_
#define CAPTURE_SERVICE_BASE_COMMON_PRODUCER_CAPTURE_EVENT_BUILDERS_H_

#include <absl/container/flat_hash_map.h>
#include <absl/time/time.h>
#include <stdint.h>

#include <string>

#include "CaptureServiceBase/CaptureServiceBase.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_service_base {

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateCaptureStartedEvent(
    const orbit_grpc_protos::CaptureOptions& capture_options, absl::Time capture_start_time,
    uint64_t capture_start_timestamp_ns);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateSuccessfulCaptureFinishedEvent();

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent
CreateInterruptedByServiceCaptureFinishedEvent(std::string message);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateFailedCaptureFinishedEvent(
    std::string message);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateClockResolutionEvent(
    uint64_t timestamp_ns, uint64_t resolution_ns);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateErrorEnablingOrbitApiEvent(
    uint64_t timestamp_ns, std::string message);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent
CreateErrorEnablingUserSpaceInstrumentationEvent(uint64_t timestamp_ns, std::string message);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent
CreateWarningInstrumentingWithUserSpaceInstrumentationEvent(
    uint64_t timestamp_ns,
    const absl::flat_hash_map<uint64_t, std::string>& function_ids_to_error_messages);

[[nodiscard]] orbit_grpc_protos::ProducerCaptureEvent CreateWarningEvent(uint64_t timestamp_ns,
                                                                         std::string message);

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_COMMON_PRODUCER_CAPTURE_EVENT_BUILDERS_H_
