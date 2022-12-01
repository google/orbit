// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_EVENT_PROCESSOR_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_H_
#define CAPTURE_EVENT_PROCESSOR_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <google/protobuf/arena.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/sync_stream.h>
#include <grpcpp/support/sync_stream.h>
#include <stdint.h>

#include <memory>
#include <thread>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.pb.h"
#include "ProducerEventProcessor/ClientCaptureEventCollector.h"

namespace orbit_producer_event_processor {

// This class receives the ClientCaptureEvents emitted by a ProducerEventProcessor and continuously
// sends them to the client buffered in CaptureResponses.
class GrpcClientCaptureEventCollector final : public ClientCaptureEventCollector {
 public:
  explicit GrpcClientCaptureEventCollector(
      grpc::ServerReaderWriterInterface<orbit_grpc_protos::CaptureResponse,
                                        orbit_grpc_protos::CaptureRequest>* reader_writer);

  void AddEvent(orbit_grpc_protos::ClientCaptureEvent&& event) override;

  void StopAndWait() override;

  ~GrpcClientCaptureEventCollector() override;

 private:
  void SenderThread();

  grpc::ServerReaderWriterInterface<orbit_grpc_protos::CaptureResponse,
                                    orbit_grpc_protos::CaptureRequest>* reader_writer_;
  absl::Mutex mutex_;
  std::thread sender_thread_;
  bool stop_requested_ ABSL_GUARDED_BY(mutex_) = false;

  std::unique_ptr<char[]> initial_block_of_first_arena_;
  std::unique_ptr<char[]> initial_block_of_second_arena_;
  std::unique_ptr<google::protobuf::Arena> arena_of_capture_responses_being_built_
      ABSL_GUARDED_BY(mutex_);
  std::vector<orbit_grpc_protos::CaptureResponse*> capture_responses_being_built_
      ABSL_GUARDED_BY(mutex_);
  std::unique_ptr<google::protobuf::Arena> arena_of_capture_responses_to_send_;
  std::vector<orbit_grpc_protos::CaptureResponse*> capture_responses_to_send_;

  uint64_t total_number_of_events_sent_ = 0;
  uint64_t total_number_of_bytes_sent_ = 0;
};

}  // namespace orbit_producer_event_processor

#endif  // CAPTURE_EVENT_PROCESSOR_GRPC_CLIENT_CAPTURE_EVENT_COLLECTOR_H_
