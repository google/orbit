// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WindowsCaptureService/CaptureServiceImpl.h"

#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include "GrpcProtos/Constants.h"
#include "Introspection/Introspection.h"
#include "ObjectUtils/CoffFile.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "TracingHandler.h"
#include "capture.pb.h"

namespace orbit_windows_capture_service {

using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::ProducerCaptureEvent;

using orbit_producer_event_processor::ProducerEventProcessor;

using orbit_producer_event_processor::GrpcClientCaptureEventCollector;

// TracingHandler::Stop is blocking until all ETW events have been processed.
// CaptureStartStopListener::OnCaptureStopRequested is also to be assumed blocking,
// for example until all CaptureEvents from external producers have been received.
// Call those methods in parallel to minimize wait time.
static void StopInternalProducersAndCaptureStartStopListenersInParallel(
    TracingHandler* tracing_handler,
    absl::flat_hash_set<CaptureStartStopListener*>* capture_start_stop_listeners) {
  std::vector<std::thread> stop_threads;

  stop_threads.emplace_back([&tracing_handler] {
    tracing_handler->Stop();
    LOG("Windows TracingHandler stopped: etw tracing is done");
  });

  for (CaptureStartStopListener* listener : *capture_start_stop_listeners) {
    stop_threads.emplace_back([&listener] {
      listener->OnCaptureStopRequested();
      LOG("CaptureStartStopListener stopped: one or more producers finished capturing");
    });
  }

  for (std::thread& stop_thread : stop_threads) {
    stop_thread.join();
  }
}

static ProducerCaptureEvent CreateCaptureStartedEvent(const CaptureOptions& capture_options,
                                                      uint64_t capture_start_timestamp_ns) {
  ProducerCaptureEvent event;
  CaptureStarted* capture_started = event.mutable_capture_started();

  uint32_t target_pid = capture_options.pid();

  capture_started->set_process_id(target_pid);

  auto executable_path_or_error = orbit_base::GetExecutablePath(target_pid);
  if (executable_path_or_error.has_value()) {
    std::filesystem::path executable_path = executable_path_or_error.value();
    capture_started->set_executable_path(executable_path.string());

    ErrorMessageOr<std::unique_ptr<orbit_object_utils::CoffFile>> coff_file_or_error =
        orbit_object_utils::CreateCoffFile(executable_path);
    if (coff_file_or_error.has_value()) {
      capture_started->set_executable_build_id(coff_file_or_error.value()->GetBuildId());
    } else {
      ERROR("Unable to load module: %s", coff_file_or_error.error().message());
    }
  } else {
    ERROR("%s", executable_path_or_error.error().message());
  }

  capture_started->mutable_capture_options()->CopyFrom(capture_options);
  capture_started->set_capture_start_timestamp_ns(capture_start_timestamp_ns);
  capture_started->mutable_capture_options()->CopyFrom(capture_options);
  return event;
}

static ProducerCaptureEvent CreateCaptureFinishedEvent() {
  ProducerCaptureEvent event;
  CaptureFinished* capture_finished = event.mutable_capture_finished();
  capture_finished->set_status(CaptureFinished::kSuccessful);
  return event;
}

grpc::Status CaptureServiceImpl::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  orbit_base::SetCurrentThreadName("CSImpl::Capture");
  if (is_capturing) {
    ERROR("Cannot start capture because another capture is already in progress");
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "Cannot start capture because another capture is already in progress.");
  }
  is_capturing = true;

  GrpcClientCaptureEventCollector client_capture_event_collector{reader_writer};
  std::unique_ptr<ProducerEventProcessor> producer_event_processor =
      ProducerEventProcessor::Create(&client_capture_event_collector);
  TracingHandler tracing_handler{producer_event_processor.get()};

  CaptureRequest request;
  reader_writer->Read(&request);
  LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");

  const CaptureOptions& capture_options = request.capture_options();

  uint64_t capture_start_timestamp_ns = orbit_base::CaptureTimestampNs();
  producer_event_processor->ProcessEvent(
      orbit_grpc_protos::kRootProducerId,
      CreateCaptureStartedEvent(capture_options, capture_start_timestamp_ns));

  tracing_handler.Start(capture_options);

  for (CaptureStartStopListener* listener : capture_start_stop_listeners_) {
    listener->OnCaptureStartRequested(request.capture_options(), producer_event_processor.get());
  }

  // The client asks for the capture to be stopped by calling WritesDone. At that point, this call
  // to Read will return false. In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  LOG("Client finished writing on Capture's gRPC stream: stopping capture");

  StopInternalProducersAndCaptureStartStopListenersInParallel(&tracing_handler,
                                                              &capture_start_stop_listeners_);

  producer_event_processor->ProcessEvent(orbit_grpc_protos::kRootProducerId,
                                         CreateCaptureFinishedEvent());

  client_capture_event_collector.StopAndWait();
  LOG("Finished handling gRPC call to Capture: all capture data has been sent");
  is_capturing = false;
  return grpc::Status::OK;
}

void CaptureServiceImpl::AddCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool new_insertion = capture_start_stop_listeners_.insert(listener).second;
  CHECK(new_insertion);
}

void CaptureServiceImpl::RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
  bool was_removed = capture_start_stop_listeners_.erase(listener) > 0;
  CHECK(was_removed);
}

}  // namespace orbit_windows_capture_service
