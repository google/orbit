// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerSideService/ProducerSideServiceImpl.h"

#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>
#include <google/protobuf/arena.h>
#include <stddef.h>

#include <thread>
#include <utility>

#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_producer_side_service {

using orbit_grpc_protos::ProducerCaptureEvent;

void ProducerSideServiceImpl::OnCaptureStartRequested(
    orbit_grpc_protos::CaptureOptions capture_options,
    orbit_producer_event_processor::ProducerEventProcessor* producer_event_processor) {
  ORBIT_CHECK(producer_event_processor != nullptr);
  ORBIT_LOG("About to send StartCaptureCommand to CaptureEventProducers (if any)");
  {
    absl::WriterMutexLock lock{&producer_event_processor_mutex_};
    producer_event_processor_ = producer_event_processor;
  }
  {
    absl::MutexLock lock{&service_state_mutex_};
    service_state_.capture_status = CaptureStatus::kCaptureStarted;
    service_state_.capture_options = std::move(capture_options);
  }
}

void ProducerSideServiceImpl::OnCaptureStopRequested() {
  ORBIT_LOG("About to send StopCaptureCommand to CaptureEventProducers (if any)");
  {
    absl::MutexLock lock{&service_state_mutex_};
    service_state_.capture_status = CaptureStatus::kCaptureStopping;

    // Wait (for a limited amount of time) for all producers to send AllEventsSent or to disconnect.
    service_state_mutex_.AwaitWithTimeout(
        absl::Condition(
            +[](ServiceState* service_state) {
              return service_state->producers_remaining == 0 || service_state->exit_requested;
            },
            &service_state_),
        absl::Milliseconds(static_cast<int64_t>(max_wait_for_all_events_sent_ms_)));
    ORBIT_CHECK(service_state_.producers_remaining >= 0);
    if (service_state_.producers_remaining == 0) {
      ORBIT_LOG("All CaptureEventProducers have finished sending their CaptureEvents");
    } else {
      ORBIT_ERROR(
          "Stopped receiving CaptureEvents from CaptureEventProducers "
          "even if not all have sent all their CaptureEvents");
    }
    ORBIT_LOG("About to send CaptureFinishedCommand to CaptureEventProducers (if any)");
    service_state_.capture_status = CaptureStatus::kCaptureFinished;
    service_state_.capture_options = std::nullopt;
    service_state_.producers_remaining = 0;
  }

  {
    absl::WriterMutexLock lock{&producer_event_processor_mutex_};
    producer_event_processor_ = nullptr;
  }
}

void ProducerSideServiceImpl::OnExitRequest() {
  {
    absl::MutexLock lock{&service_state_mutex_};
    service_state_.exit_requested = true;
    service_state_.capture_options = std::nullopt;
  }

  ORBIT_LOG("Attempting to disconnect from CaptureEventProducers as exit was requested");
  {
    absl::MutexLock lock{&server_contexts_mutex_};
    for (grpc::ServerContext* context : server_contexts_) {
      // This should cause blocking Reads on ServerReaderWriter to fail immediately.
      context->TryCancel();
    }
  }

  {
    absl::WriterMutexLock lock{&producer_event_processor_mutex_};
    producer_event_processor_ = nullptr;
  }
}

grpc::Status ProducerSideServiceImpl::ReceiveCommandsAndSendEvents(
    ::grpc::ServerContext* context,
    ::grpc::ServerReaderWriter< ::orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                                ::orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream) {
  ORBIT_LOG("A CaptureEventProducer has connected calling ReceiveCommandsAndSendEvents");

  {
    absl::MutexLock lock{&server_contexts_mutex_};
    server_contexts_.emplace(context);
  }

  // This keeps whether we are still waiting for an AllEventsSent message at the end of a capture.
  // It starts as true as we aren't yet waiting for such message when the connection is established.
  // Note that this will also be protected by service_state_mutex_.
  bool all_events_sent_received = true;

  std::atomic<bool> receive_events_thread_exited = false;

  // This thread is responsible for writing on stream, and specifically for
  // sending StartCaptureCommands and StopCaptureCommands to the connected producer.
  std::thread send_commands_thread{&ProducerSideServiceImpl::SendCommandsThread,
                                   this,
                                   context,
                                   stream,
                                   &all_events_sent_received,
                                   &receive_events_thread_exited};

  // This thread is responsible for reading from stream, and specifically for
  // receiving ProducerCaptureEvents and AllEventsSent messages.
  std::thread receive_events_thread{&ProducerSideServiceImpl::ReceiveEventsThread,
                                    this,
                                    context,
                                    stream,
                                    producer_id_counter_++,
                                    &all_events_sent_received};
  receive_events_thread.join();

  // When receive_events_thread exits because stream->Read(&request) fails,
  // it means that the producer has disconnected: ask send_commands_thread to exit, too.
  receive_events_thread_exited = true;
  send_commands_thread.join();

  {
    absl::MutexLock lock{&server_contexts_mutex_};
    server_contexts_.erase(context);
  }

  ORBIT_LOG("Finished handling ReceiveCommandsAndSendEvents for a CaptureEventProducer");
  return grpc::Status::OK;
}

static bool SendStartCaptureCommand(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                             orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream,
    orbit_grpc_protos::CaptureOptions capture_options) {
  orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
  *command.mutable_start_capture_command()->mutable_capture_options() = std::move(capture_options);
  if (!stream->Write(command)) {
    ORBIT_ERROR("Sending StartCaptureCommand to CaptureEventProducer");
    ORBIT_LOG("Terminating call to ReceiveCommandsAndSendEvents as Write failed");
    // Cause Read in ReceiveEventsThread to also fail if for some reason it hasn't already.
    context->TryCancel();
    return false;
  }
  ORBIT_LOG("Sent StartCaptureCommand to CaptureEventProducer");
  return true;
}

static bool SendStopCaptureCommand(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                             orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream) {
  orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
  command.mutable_stop_capture_command();
  if (!stream->Write(command)) {
    ORBIT_ERROR("Sending StopCaptureCommand to CaptureEventProducer");
    ORBIT_LOG("Terminating call to ReceiveCommandsAndSendEvents as Write failed");
    // Cause Read in ReceiveEventsThread to also fail if for some reason it hasn't already.
    context->TryCancel();
    return false;
  }
  ORBIT_LOG("Sent StopCaptureCommand to CaptureEventProducer");
  return true;
}

static bool SendCaptureFinishedCommand(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                             orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream) {
  orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
  command.mutable_capture_finished_command();
  if (!stream->Write(command)) {
    ORBIT_ERROR("Sending CaptureFinishedCommand to CaptureEventProducer");
    ORBIT_LOG("Terminating call to ReceiveCommandsAndSendEvents as Write failed");
    // Cause Read in ReceiveEventsThread to also fail if for some reason it hasn't already.
    context->TryCancel();
    return false;
  }
  ORBIT_LOG("Sent CaptureFinishedCommand to CaptureEventProducer");
  return true;
}

void ProducerSideServiceImpl::SendCommandsThread(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                             orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream,
    bool* all_events_sent_received, std::atomic<bool>* receive_events_thread_exited) {
  // As a result of initializing prev_capture_status to kCaptureFinished,
  // an initial StartCaptureCommand is sent
  // if service_state_.capture_status is actually CaptureStatus::kCaptureStarted,
  // and an initial StopCaptureCommand is sent (with little effect)
  // if service_state_.capture_status is actually CaptureStatus::kCaptureStopping.
  CaptureStatus prev_capture_status = CaptureStatus::kCaptureFinished;

  // This loop keeps track of changes to service_state_.capture_status using
  // conditional critical sections on service_state_mutex_ and updating prev_capture_status,
  // and sends StartCaptureCommands and StopCaptureCommands accordingly.
  // It exits when one of *receive_events_thread_exited and service_state_.exit_requested is true,
  // or when Write fails (because the producer disconnected or because the context was cancelled).
  while (true) {
    // This is set when ReceiveEventsThread has exited. At that point this thread should also exit.
    if (*receive_events_thread_exited) {
      return;
    }

    CaptureStatus curr_capture_status;
    std::optional<orbit_grpc_protos::CaptureOptions> curr_capture_options;
    {
      absl::MutexLock lock{&service_state_mutex_};
      if (service_state_.exit_requested) {
        return;
      }

      if (service_state_.capture_status == prev_capture_status) {
        // Wait for service_state_.capture_status to change or for service_state->exit_requested
        // (the next iteration will handle the change).
        // Use a timeout to periodically check (in the next iteration)
        // for *terminate_send_commands_thread, set by ReceiveCommandsAndSendEvents.
        static constexpr absl::Duration kCheckExitSendCommandsThreadInterval = absl::Seconds(1);

        // The three cases in this switch are almost identical, except
        // for the value service_state->capture_status is compared with.
        // The reason for the duplication is that AwaitWithTimeout takes a function pointer, which
        // means that the lambda cannot capture the initial value of service_state_.capture_status.
        switch (service_state_.capture_status) {
          case CaptureStatus::kCaptureStarted: {
            service_state_mutex_.AwaitWithTimeout(absl::Condition(
                                                      +[](ServiceState* service_state) {
                                                        return service_state->exit_requested ||
                                                               service_state->capture_status !=
                                                                   CaptureStatus::kCaptureStarted;
                                                      },
                                                      &service_state_),
                                                  kCheckExitSendCommandsThreadInterval);
          } break;

          case CaptureStatus::kCaptureStopping: {
            service_state_mutex_.AwaitWithTimeout(absl::Condition(
                                                      +[](ServiceState* service_state) {
                                                        return service_state->exit_requested ||
                                                               service_state->capture_status !=
                                                                   CaptureStatus::kCaptureStopping;
                                                      },
                                                      &service_state_),
                                                  kCheckExitSendCommandsThreadInterval);
          } break;

          case CaptureStatus::kCaptureFinished: {
            service_state_mutex_.AwaitWithTimeout(absl::Condition(
                                                      +[](ServiceState* service_state) {
                                                        return service_state->exit_requested ||
                                                               service_state->capture_status !=
                                                                   CaptureStatus::kCaptureFinished;
                                                      },
                                                      &service_state_),
                                                  kCheckExitSendCommandsThreadInterval);
          } break;
        }

        continue;
      }

      // service_state_.capture_status has changed compared to prev_capture_status: handle the
      // change while holding service_state_mutex_ (that also protects all_events_sent_received).
      switch (service_state_.capture_status) {
        case CaptureStatus::kCaptureStarted: {
          ++service_state_.producers_remaining;
          *all_events_sent_received = false;
        } break;

        case CaptureStatus::kCaptureStopping:
          break;

        case CaptureStatus::kCaptureFinished: {
          *all_events_sent_received = true;
        } break;
      }
      curr_capture_status = service_state_.capture_status;
      curr_capture_options = service_state_.capture_options;
    }  // absl::MutexLock lock{&service_state_mutex_}

    // curr_capture_status now holds the new service_state_.capture_status. Send commands
    // to the producer based on its value and also based on the value of prev_capture_status,
    // in case this thread missed an intermediate change of service_state_.capture_status.
    switch (curr_capture_status) {
      case CaptureStatus::kCaptureStarted: {
        ORBIT_CHECK(curr_capture_options.has_value());
        if (prev_capture_status == CaptureStatus::kCaptureFinished) {
          if (!SendStartCaptureCommand(context, stream, curr_capture_options.value())) {
            return;
          }
        } else if (prev_capture_status == CaptureStatus::kCaptureStopping) {
          if (!SendCaptureFinishedCommand(context, stream) ||
              !SendStartCaptureCommand(context, stream, curr_capture_options.value())) {
            return;
          }
        } else {
          ORBIT_UNREACHABLE();
        }
      } break;

      case CaptureStatus::kCaptureStopping: {
        ORBIT_CHECK(curr_capture_options.has_value());
        if (prev_capture_status == CaptureStatus::kCaptureStarted) {
          if (!SendStopCaptureCommand(context, stream)) {
            return;
          }
        } else if (prev_capture_status == CaptureStatus::kCaptureFinished) {
          if (!SendStartCaptureCommand(context, stream, curr_capture_options.value()) ||
              !SendStopCaptureCommand(context, stream)) {
            return;
          }
        } else {
          ORBIT_UNREACHABLE();
        }
      } break;

      case CaptureStatus::kCaptureFinished:
        ORBIT_CHECK(!curr_capture_options.has_value());
        if (prev_capture_status == CaptureStatus::kCaptureStopping) {
          if (!SendCaptureFinishedCommand(context, stream)) {
            return;
          }
        } else if (prev_capture_status == CaptureStatus::kCaptureStarted) {
          if (!SendStopCaptureCommand(context, stream) ||
              !SendCaptureFinishedCommand(context, stream)) {
            return;
          }
        } else {
          ORBIT_UNREACHABLE();
        }
        break;
    }

    prev_capture_status = curr_capture_status;
  }
}

void ProducerSideServiceImpl::ReceiveEventsThread(
    grpc::ServerContext* /*context*/,
    grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                             orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream,
    uint64_t producer_id, bool* all_events_sent_received) {
  orbit_base::SetCurrentThreadName("PSSI::RcvEvents");

  // Create the ReceiveCommandsAndSendEventsRequest on a protobuf Arena, which prevents memory
  // allocations for the multiple sub-messages containing ProducerCaptureEvents. Now, when calling
  // ProducerEventProcessor::ProcessEvent below, passing the ProducerCaptureEvent will result in a
  // deep copy and the memory allocations we saved before will happen here, plus copies.
  // Nonetheless, and maybe counterintuitively, the performance improvement we measured is huge. In
  // particular, ServerReaderWriter::Read seems particularly inefficient without an Arena.
  google::protobuf::ArenaOptions arena_options;
  constexpr size_t kArenaFixedBlockSize = 1024 * 1024;
  auto arena_initial_block = make_unique_for_overwrite<char[]>(kArenaFixedBlockSize);
  arena_options.initial_block = arena_initial_block.get();
  arena_options.initial_block_size = kArenaFixedBlockSize;
  arena_options.start_block_size = kArenaFixedBlockSize;
  arena_options.max_block_size = kArenaFixedBlockSize;

  while (true) {
    google::protobuf::Arena arena{arena_options};
    auto* request = google::protobuf::Arena::CreateMessage<
        orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>(&arena);
    if (!stream->Read(request)) break;

    {
      absl::MutexLock lock{&service_state_mutex_};
      if (service_state_.exit_requested) {
        break;
      }
    }

    switch (request->event_case()) {
      case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kBufferedCaptureEvents: {
        // We use ReaderMutexLock because the mutex guards the value of producer_event_processor_,
        // it does not guard calls to ProcessEvent nor the internal state of the object implementing
        // the interface. The interface implementation is by itself thread-safe.
        absl::ReaderMutexLock lock{&producer_event_processor_mutex_};
        // producer_event_processor_ can be nullptr if a producer sends events while not capturing.
        // Don't log an error in such a case as it could easily spam the logs.
        if (producer_event_processor_ != nullptr) {
          for (ProducerCaptureEvent& event :
               *request->mutable_buffered_capture_events()->mutable_capture_events()) {
            producer_event_processor_->ProcessEvent(producer_id, std::move(event));
          }
        }
      } break;

      case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kAllEventsSent: {
        ORBIT_LOG("Received AllEventsSent from CaptureEventProducer");
        absl::MutexLock lock{&service_state_mutex_};
        switch (service_state_.capture_status) {
          case CaptureStatus::kCaptureStarted: {
            ORBIT_ERROR("CaptureEventProducer sent AllEventsSent while still capturing");
            // Even if we weren't waiting for the AllEventsSent message yet,
            // still keep track of the fact that we have already received it.
            if (!*all_events_sent_received) {
              --service_state_.producers_remaining;
              *all_events_sent_received = true;
            }
          } break;

          case CaptureStatus::kCaptureStopping: {
            // If we were waiting for AllEventsSent, keep track of the fact that we received it.
            if (!*all_events_sent_received) {
              --service_state_.producers_remaining;
              *all_events_sent_received = true;
            }
          } break;

          case CaptureStatus::kCaptureFinished: {
            ORBIT_ERROR("CaptureEventProducer sent AllEventsSent after the capture had finished");
          } break;
        }
      } break;

      case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::EVENT_NOT_SET: {
        ORBIT_ERROR("CaptureEventProducer sent EVENT_NOT_SET");
      } break;
    }
  }

  ORBIT_ERROR("Receiving ReceiveCommandsAndSendEventsRequest from CaptureEventProducer");
  {
    absl::MutexLock lock{&service_state_mutex_};
    // Producer has disconnected: treat this as if it had sent all its CaptureEvents.
    if (!*all_events_sent_received &&
        (service_state_.capture_status == CaptureStatus::kCaptureStarted ||
         service_state_.capture_status == CaptureStatus::kCaptureStopping)) {
      --service_state_.producers_remaining;
      *all_events_sent_received = true;
    }
  }
}

}  // namespace orbit_producer_side_service
