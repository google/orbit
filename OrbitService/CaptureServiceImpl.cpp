// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceImpl.h"

#include "CaptureEventBuffer.h"
#include "LinuxTracingHandler.h"
#include "OrbitBase/Logging.h"

namespace orbit_service {

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

namespace {

using orbit_grpc_protos::CaptureEvent;

class CaptureEventBufferAndResponseSender final : public CaptureEventBuffer {
 public:
  explicit CaptureEventBufferAndResponseSender(
      grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {
    CHECK(reader_writer_ != nullptr);
    sender_thread_ = std::thread{[this] { SenderThread(); }};
  }

  void AddEvent(orbit_grpc_protos::CaptureEvent&& event) override {
    absl::MutexLock lock{&event_buffer_mutex_};
    // Protect stop_requested_ with event_buffer_mutex_ so that we can use stop_requested_
    // in Conditions for Await/LockWhen (specifically, in SenderThread).
    if (stop_requested_) {
      return;
    }
    event_buffer_.emplace_back(std::move(event));
  }

  void StopAndWait() {
    CHECK(sender_thread_.joinable());
    {
      absl::MutexLock lock{&event_buffer_mutex_};
      stop_requested_ = true;
    }
    sender_thread_.join();
  }

  ~CaptureEventBufferAndResponseSender() override { CHECK(!sender_thread_.joinable()); }

 private:
  void SenderThread() {
    pthread_setname_np(pthread_self(), "SenderThread");
    constexpr absl::Duration kSendTimeInterval = absl::Milliseconds(20);
    // This should be lower than kMaxEventsPerResponse in SendBufferedEvents as
    // a few more events are likely to arrive after the condition becomes true.
    constexpr uint64_t kSendEventCountInterval = 5000;

    bool stopped = false;
    while (!stopped) {
      ORBIT_SCOPE("SenderThread iteration");
      event_buffer_mutex_.LockWhenWithTimeout(absl::Condition(
                                                  +[](CaptureEventBufferAndResponseSender* self) {
                                                    return self->event_buffer_.size() >=
                                                               kSendEventCountInterval ||
                                                           self->stop_requested_;
                                                  },
                                                  this),
                                              kSendTimeInterval);
      if (stop_requested_) {
        stopped = true;
      }
      std::vector<CaptureEvent> buffered_events = std::move(event_buffer_);
      event_buffer_.clear();
      event_buffer_mutex_.Unlock();
      SendBufferedEvents(std::move(buffered_events));
    }
  }

  void SendBufferedEvents(std::vector<CaptureEvent>&& events) {
    ORBIT_SCOPE_FUNCTION;
    ORBIT_UINT64("Number of sent buffered events", events.size());
    if (events.empty()) {
      return;
    }

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

  std::vector<orbit_grpc_protos::CaptureEvent> event_buffer_;
  absl::Mutex event_buffer_mutex_;
  grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer_;
  std::thread sender_thread_;
  bool stop_requested_ = false;
};

}  // namespace

grpc::Status CaptureServiceImpl::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  pthread_setname_np(pthread_self(), "CSImpl::Capture");
  CaptureEventBufferAndResponseSender buffer_and_sender{reader_writer};
  LinuxTracingHandler tracing_handler{&buffer_and_sender};

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
  LOG("LinuxTracingHandler stopped: perf_event_open tracing is done");

  buffer_and_sender.StopAndWait();
  LOG("Finished handling gRPC call to Capture: all capture data has been sent");
  return grpc::Status::OK;
}

}  // namespace orbit_service
