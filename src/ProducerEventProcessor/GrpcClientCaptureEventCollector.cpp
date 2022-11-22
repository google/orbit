// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"

#include <absl/time/time.h>
#include <google/protobuf/arena.h>
#include <stddef.h>

#include <algorithm>
#include <utility>

#include "ApiInterface/Orbit.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/ThreadUtils.h"

using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_producer_event_processor {

static void InitializeArenaOfCaptureResponses(
    std::unique_ptr<google::protobuf::Arena>* arena_of_capture_responses,
    std::unique_ptr<char[]>* initial_block_of_arena) {
  constexpr size_t kArenaFixedBlockSize = 1024 * 1024;
  google::protobuf::ArenaOptions arena_options;
  *initial_block_of_arena = make_unique_for_overwrite<char[]>(kArenaFixedBlockSize);
  arena_options.initial_block = initial_block_of_arena->get();
  arena_options.initial_block_size = kArenaFixedBlockSize;
  arena_options.start_block_size = kArenaFixedBlockSize;
  arena_options.max_block_size = kArenaFixedBlockSize;
  *arena_of_capture_responses = std::make_unique<google::protobuf::Arena>(arena_options);
}

GrpcClientCaptureEventCollector::GrpcClientCaptureEventCollector(
    grpc::ServerReaderWriterInterface<orbit_grpc_protos::CaptureResponse,
                                      orbit_grpc_protos::CaptureRequest>* reader_writer)
    : reader_writer_{reader_writer} {
  ORBIT_CHECK(reader_writer_ != nullptr);

  InitializeArenaOfCaptureResponses(&arena_of_capture_responses_being_built_,
                                    &initial_block_of_first_arena_);
  InitializeArenaOfCaptureResponses(&arena_of_capture_responses_to_send_,
                                    &initial_block_of_second_arena_);

  sender_thread_ = std::thread{[this] { SenderThread(); }};
}

void GrpcClientCaptureEventCollector::AddEvent(ClientCaptureEvent&& event) {
  absl::MutexLock lock{&mutex_};
  if (stop_requested_) {
    return;
  }

  // We group several ClientCaptureEvents in a single CaptureResponse to avoid sending countless
  // tiny messages. But we also want to avoid huge messages, which:
  // - would cause the capture on the client to jump forward in time in few big steps and not look
  //   live anymore;
  // - could exceed the maximum gRPC message size.
  static constexpr int kMaxEventsPerCaptureResponse = 10'000;
  if (capture_responses_being_built_.empty() ||
      capture_responses_being_built_.back()->capture_events_size() ==
          kMaxEventsPerCaptureResponse) {
    auto* capture_response = google::protobuf::Arena::CreateMessage<CaptureResponse>(
        arena_of_capture_responses_being_built_.get());
    capture_responses_being_built_.push_back(capture_response);
  }
  capture_responses_being_built_.back()->mutable_capture_events()->Add(std::move(event));
}

void GrpcClientCaptureEventCollector::StopAndWait() {
  ORBIT_CHECK(sender_thread_.joinable());
  {
    // Protect stop_requested_ with mutex_ so that we can use stop_requested_ in Conditions for
    // Await/LockWhen (specifically, in SenderThread).
    absl::MutexLock lock{&mutex_};
    stop_requested_ = true;
  }
  sender_thread_.join();
}

GrpcClientCaptureEventCollector::~GrpcClientCaptureEventCollector() {
  ORBIT_CHECK(!sender_thread_.joinable());

  ORBIT_LOG("Total number of events sent: %u", total_number_of_events_sent_);
  ORBIT_LOG("Total number of bytes sent: %u", total_number_of_bytes_sent_);

  if (total_number_of_events_sent_ > 0) {
    float average_bytes = static_cast<float>(total_number_of_bytes_sent_) /
                          static_cast<float>(total_number_of_events_sent_);
    ORBIT_LOG("Average number of bytes per event: %.2f", average_bytes);
  }
}

void GrpcClientCaptureEventCollector::SenderThread() {
  orbit_base::SetCurrentThreadName("SenderThread");
  constexpr absl::Duration kSendTimeInterval = absl::Milliseconds(20);

  bool stopped = false;
  while (!stopped) {
    ORBIT_SCOPE("SenderThread iteration");

    mutex_.LockWhenWithTimeout(
        absl::Condition(
            +[](GrpcClientCaptureEventCollector* self) ABSL_EXCLUSIVE_LOCKS_REQUIRED(self->mutex_) {
              // This should be lower than (not equal to) kMaxEventsPerCaptureResponse in AddEvent
              // as a few more ClientCaptureEvents are likely to arrive after the condition becomes
              // true.
              constexpr int kSendEventCountInterval = 5000;

              return (self->capture_responses_being_built_.size() == 1 &&
                      self->capture_responses_being_built_.back()->capture_events_size() >=
                          kSendEventCountInterval) ||
                     self->capture_responses_being_built_.size() > 1 || self->stop_requested_;
            },
            this),
        kSendTimeInterval);
    if (stop_requested_) {
      stopped = true;
    }
    if (capture_responses_being_built_.empty()) {
      mutex_.Unlock();
      continue;
    }

    // We employ double buffering, and the two Arenas `arena_of_capture_response_being_built_` and
    // `arena_of_capture_response_to_send_` are effectively the two buffers.
    arena_of_capture_responses_being_built_.swap(arena_of_capture_responses_to_send_);
    capture_responses_being_built_.swap(capture_responses_to_send_);
    mutex_.Unlock();

    uint64_t number_of_events_sent = 0;
    uint64_t number_of_bytes_sent = 0;

    // Note that usually we only have one CaptureResponse to send because kSendEventCountInterval is
    // quite lower than kMaxEventsPerCaptureResponse. But we can have more than one if new events
    // come faster than `reader_writer_->Write` executes, which can for example happen if the client
    // is a bit unresponsive.
    for (CaptureResponse* capture_response : capture_responses_to_send_) {
      // Record statistics on event count and byte size for this CaptureResponse.
      int capture_response_event_count = capture_response->capture_events_size();
      ORBIT_CHECK(capture_response_event_count > 0);
      ORBIT_INT("Number of CaptureEvents in CaptureResponse", capture_response_event_count);

      uint64_t capture_response_bytes = capture_response->ByteSizeLong();
      ORBIT_INT("Byte size of CaptureResponse", capture_response_bytes);

      number_of_events_sent += capture_response_event_count;
      number_of_bytes_sent += capture_response_bytes;

      // Now send the CaptureResponse.
      {
        ORBIT_SCOPE("reader_writer_->Write");
        reader_writer_->Write(*capture_response);
      }
    }

    // Record statistics on event count and byte size for this entire iteration.
    {
      ORBIT_UINT64("Number of buffered CaptureEvents sent", number_of_events_sent);

      ORBIT_CHECK(number_of_events_sent > 0);
      [[maybe_unused]] const float average_bytes =
          static_cast<float>(number_of_bytes_sent) / static_cast<float>(number_of_events_sent);
      ORBIT_FLOAT("Average bytes per CaptureEvent", average_bytes);

      total_number_of_events_sent_ += number_of_events_sent;
      total_number_of_bytes_sent_ += number_of_bytes_sent;
    }

    capture_responses_to_send_.clear();
    arena_of_capture_responses_to_send_->Reset();
  }
}

}  // namespace orbit_producer_event_processor
