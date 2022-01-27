// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CAPTURE_SERVICE_UTILS_H_
#define CAPTURE_SERVICE_CAPTURE_SERVICE_UTILS_H_

#include <absl/container/flat_hash_set.h>

#include "CaptureStartStopListener.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

namespace orbit_capture_service {

struct CaptureServiceMetaData {
  void Init(std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
                client_capture_event_collector_original) {
    client_capture_event_collector.reset(client_capture_event_collector_original.release());
    producer_event_processor = orbit_producer_event_processor::ProducerEventProcessor::Create(
        client_capture_event_collector.get());
  }

  void AddCaptureStartStopListener(CaptureStartStopListener* listener) {
    bool new_insertion = capture_start_stop_listeners.insert(listener).second;
    ORBIT_CHECK(new_insertion);
  }

  void RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
    bool was_removed = capture_start_stop_listeners.erase(listener) > 0;
    ORBIT_CHECK(was_removed);
  }

  void Reset() {
    ORBIT_CHECK(producer_event_processor != nullptr);
    ORBIT_CHECK(client_capture_event_collector != nullptr);

    producer_event_processor.reset();
    client_capture_event_collector.reset();
    capture_start_timestamp_ns = 0;
  }

  std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
      client_capture_event_collector;
  std::unique_ptr<orbit_producer_event_processor::ProducerEventProcessor> producer_event_processor;
  absl::flat_hash_set<CaptureStartStopListener*> capture_start_stop_listeners;
  uint64_t capture_start_timestamp_ns = 0;
  uint64_t clock_resolution_ns = 0;
};

void StartEventProcessing(const orbit_grpc_protos::CaptureOptions& capture_options,
                          CaptureServiceMetaData& meta_data);

enum class StopCaptureReason { kClientStop, kMemoryWatchdog };
void FinalizeEventProcessing(StopCaptureReason stop_capture_reason,
                             CaptureServiceMetaData& meta_data);

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CAPTURE_SERVICE_UTILS_H_
