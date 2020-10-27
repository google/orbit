// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceImpl.h"

#include "CaptureResponseListener.h"
#include "LinuxTracingGrpcHandler.h"
#include "OrbitBase/Logging.h"

namespace orbit_service {

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

namespace {

using orbit_grpc_protos::CaptureEvent;

class GrpcCaptureResponseSender final : public CaptureResponseListener {
 public:
  explicit GrpcCaptureResponseSender(
      grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {}

  void ProcessEvents(std::vector<CaptureEvent>&& events) override {
    if (events.empty()) {
      return;
    }

    ORBIT_SCOPE("GrpcCaptureResponseSender::ProcessEvents");
    ORBIT_UINT64("Number of sent buffered events", events.size());
    constexpr uint64_t kMaxEventsPerResponse = 10'000;
    CaptureResponse response;
    for (CaptureEvent& event : events) {
      // We buffer to avoid sending countless tiny messages, but we also want to
      // avoid huge messages, which would cause the capture on the client to jump
      // forward in time in few big steps and not look live anymore.
      if (response.capture_events_size() == kMaxEventsPerResponse) {
        reader_writer_->Write(response);
        response.clear_capture_events();
      }
      response.mutable_capture_events()->Add(std::move(event));
    }
    reader_writer_->Write(response);
  }

 private:
  grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer_;
};

}  // namespace

grpc::Status CaptureServiceImpl::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  pthread_setname_np(pthread_self(), "CSImpl::Capture");
  GrpcCaptureResponseSender capture_response_listener{reader_writer};
  LinuxTracingGrpcHandler tracing_handler{&capture_response_listener};

  CaptureRequest request;
  reader_writer->Read(&request);
  LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");
  tracing_handler.Start(std::move(*request.mutable_capture_options()));

  // The client asks for the capture to be stopped by calling WritesDone.
  // At that point, this call to Read will return false.
  // In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  LOG("Client finished writing on Capture's gRPC stream: stopping capture");
  tracing_handler.Stop();

  LOG("Finished handling gRPC call to Capture: all capture data has been sent");
  return grpc::Status::OK;
}

}  // namespace orbit_service
