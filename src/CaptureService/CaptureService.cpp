// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureService/CaptureService.h"

#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include "ApiUtils/Event.h"
#include "CaptureService/ProducerCaptureEventBuilder.h"
#include "GrpcProtos/Constants.h"
#include "Introspection/Introspection.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/Profiling.h"
#include "OrbitVersion/OrbitVersion.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"
#include "capture.pb.h"

using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::GrpcClientCaptureEventCollector;
using orbit_producer_event_processor::ProducerEventProcessor;

namespace orbit_capture_service {

void CaptureService::AddCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool new_insertion = capture_start_stop_listeners_.insert(listener).second;
  CHECK(new_insertion);
}

void CaptureService::RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool was_removed = capture_start_stop_listeners_.erase(listener) > 0;
  CHECK(was_removed);
}

void CaptureService::EstimateAndLogClockResolution() {
  // We expect the value to be small, ~35 nanoseconds.
  clock_resolution_ns_ = orbit_base::EstimateClockResolution();
  if (clock_resolution_ns_ > 0) {
    LOG("Clock resolution: %d (ns)", clock_resolution_ns_);
  } else {
    ERROR("Failed to estimate clock resolution");
  }
}

}  // namespace orbit_capture_service
