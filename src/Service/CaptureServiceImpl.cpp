// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceImpl.h"

#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <pthread.h>
#include <stdint.h>

#include <algorithm>
#include <limits>
#include <thread>
#include <utility>
#include <vector>

#include "CaptureEventBuffer.h"
#include "CaptureEventSender.h"
#include "LinuxTracingHandler.h"
#include "MemoryInfoHandler.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"
#include "ProducerEventProcessor.h"
#include "capture.pb.h"

namespace orbit_service {

using orbit_grpc_protos::CaptureRequest;
using orbit_grpc_protos::CaptureResponse;

namespace {

using orbit_grpc_protos::ClientCaptureEvent;

class SenderThreadCaptureEventBuffer final : public CaptureEventBuffer {
 public:
  explicit SenderThreadCaptureEventBuffer(CaptureEventSender* event_sender)
      : capture_event_sender_{event_sender} {
    CHECK(capture_event_sender_ != nullptr);
    sender_thread_ = std::thread{[this] { SenderThread(); }};
  }

  void AddEvent(ClientCaptureEvent&& event) override {
    absl::MutexLock lock{&event_buffer_mutex_};
    if (stop_requested_) {
      return;
    }
    event_buffer_.emplace_back(std::move(event));
  }

  void StopAndWait() {
    CHECK(sender_thread_.joinable());
    {
      // Protect stop_requested_ with event_buffer_mutex_ so that we can use stop_requested_
      // in Conditions for Await/LockWhen (specifically, in SenderThread).
      absl::MutexLock lock{&event_buffer_mutex_};
      stop_requested_ = true;
    }
    sender_thread_.join();
  }

  ~SenderThreadCaptureEventBuffer() override { CHECK(!sender_thread_.joinable()); }

 private:
  void SenderThread() {
    pthread_setname_np(pthread_self(), "SenderThread");
    constexpr absl::Duration kSendTimeInterval = absl::Milliseconds(20);
    // This should be lower than kMaxEventsPerResponse in GrpcCaptureEventSender::SendEvents
    // as a few more events are likely to arrive after the condition becomes true.
    constexpr uint64_t kSendEventCountInterval = 5000;

    bool stopped = false;
    while (!stopped) {
      ORBIT_SCOPE("SenderThread iteration");
      event_buffer_mutex_.LockWhenWithTimeout(absl::Condition(
                                                  +[](SenderThreadCaptureEventBuffer* self) {
                                                    return self->event_buffer_.size() >=
                                                               kSendEventCountInterval ||
                                                           self->stop_requested_;
                                                  },
                                                  this),
                                              kSendTimeInterval);
      if (stop_requested_) {
        stopped = true;
      }
      std::vector<ClientCaptureEvent> buffered_events = std::move(event_buffer_);
      event_buffer_.clear();
      event_buffer_mutex_.Unlock();
      capture_event_sender_->SendEvents(std::move(buffered_events));
    }
  }

  std::vector<ClientCaptureEvent> event_buffer_;
  absl::Mutex event_buffer_mutex_;
  CaptureEventSender* capture_event_sender_;
  std::thread sender_thread_;
  bool stop_requested_ = false;
};

class GrpcCaptureEventSender final : public CaptureEventSender {
 public:
  explicit GrpcCaptureEventSender(
      grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {
    CHECK(reader_writer_ != nullptr);
  }

  ~GrpcCaptureEventSender() override {
    LOG("Total number of events sent: %lu", total_number_of_events_sent_);
    LOG("Total number of bytes sent: %lu", total_number_of_bytes_sent_);

    // Ensure we can divide by 0.f safely.
    static_assert(std::numeric_limits<float>::is_iec559);
    float average_bytes =
        static_cast<float>(total_number_of_bytes_sent_) / total_number_of_events_sent_;

    LOG("Average number of bytes per event: %.2f", average_bytes);
  }

  void SendEvents(std::vector<ClientCaptureEvent>&& events) override {
    ORBIT_SCOPE_FUNCTION;
    ORBIT_UINT64("Number of buffered events sent", events.size());
    if (events.empty()) {
      return;
    }

    constexpr uint64_t kMaxEventsPerResponse = 10'000;
    uint64_t number_of_bytes_sent = 0;
    CaptureResponse response;
    for (ClientCaptureEvent& event : events) {
      // We buffer to avoid sending countless tiny messages, but we also want to
      // avoid huge messages, which would cause the capture on the client to jump
      // forward in time in few big steps and not look live anymore.
      if (response.capture_events_size() == kMaxEventsPerResponse) {
        number_of_bytes_sent += response.ByteSizeLong();
        reader_writer_->Write(response);
        response.clear_capture_events();
      }
      response.mutable_capture_events()->Add(std::move(event));
    }
    number_of_bytes_sent += response.ByteSizeLong();
    reader_writer_->Write(response);

    // Ensure we can divide by 0.f safely.
    static_assert(std::numeric_limits<float>::is_iec559);
    ORBIT_FLOAT("Average bytes per CaptureEvent",
                static_cast<float>(number_of_bytes_sent) / events.size());
    total_number_of_events_sent_ += events.size();
    total_number_of_bytes_sent_ += number_of_bytes_sent;
  }

 private:
  grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer_;

  uint64_t total_number_of_events_sent_ = 0;
  uint64_t total_number_of_bytes_sent_ = 0;
};

}  // namespace

// LinuxTracingHandler::Stop is blocking, until all perf_event_open events have been processed
// and all perf_event_open file descriptors have been closed.
// CaptureStartStopListener::OnCaptureStopRequested is also to be assumed blocking,
// for example until all CaptureEvents from external producers have been received.
// Hence why these methods need to be called in parallel on different threads.
static void StopInternalProducersAndCaptureStartStopListenersInParallel(
    LinuxTracingHandler* tracing_handler, MemoryInfoHandler* memory_info_handler,
    absl::flat_hash_set<CaptureStartStopListener*>* capture_start_stop_listeners) {
  std::vector<std::thread> stop_threads;

  stop_threads.emplace_back([&tracing_handler] {
    tracing_handler->Stop();
    LOG("LinuxTracingHandler stopped: perf_event_open tracing is done");
  });

  stop_threads.emplace_back([&memory_info_handler] {
    memory_info_handler->Stop();
    LOG("MemoryInfoHandler stopped: total memory usage information collection is done");
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

grpc::Status CaptureServiceImpl::Capture(
    grpc::ServerContext*,
    grpc::ServerReaderWriter<CaptureResponse, CaptureRequest>* reader_writer) {
  pthread_setname_np(pthread_self(), "CSImpl::Capture");
  if (is_capturing) {
    ERROR("Cannot start capture because another capture is already in progress");
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS,
                        "Cannot start capture because another capture is already in progress.");
  }
  is_capturing = true;

  GrpcCaptureEventSender capture_event_sender{reader_writer};
  SenderThreadCaptureEventBuffer capture_event_buffer{&capture_event_sender};
  std::unique_ptr<ProducerEventProcessor> producer_event_processor =
      ProducerEventProcessor::Create(&capture_event_buffer);
  LinuxTracingHandler tracing_handler{producer_event_processor.get()};
  MemoryInfoHandler memory_info_handler{producer_event_processor.get()};

  CaptureRequest request;
  reader_writer->Read(&request);
  LOG("Read CaptureRequest from Capture's gRPC stream: starting capture");

  tracing_handler.Start(request.capture_options());
  memory_info_handler.Start(request.capture_options());
  for (CaptureStartStopListener* listener : capture_start_stop_listeners_) {
    listener->OnCaptureStartRequested(request.capture_options(), producer_event_processor.get());
  }

  // The client asks for the capture to be stopped by calling WritesDone.
  // At that point, this call to Read will return false.
  // In the meantime, it blocks if no message is received.
  while (reader_writer->Read(&request)) {
  }
  LOG("Client finished writing on Capture's gRPC stream: stopping capture");

  StopInternalProducersAndCaptureStartStopListenersInParallel(
      &tracing_handler, &memory_info_handler, &capture_start_stop_listeners_);

  capture_event_buffer.StopAndWait();
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

}  // namespace orbit_service
