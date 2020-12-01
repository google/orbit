// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerSideServiceImpl.h"

#include <thread>

#include "OrbitBase/Logging.h"

namespace orbit_service {

void ProducerSideServiceImpl::OnCaptureStartRequested(CaptureEventBuffer* capture_event_buffer) {
  CHECK(capture_event_buffer != nullptr);
  LOG("About to send StartCaptureCommand to CaptureEventProducers (if any)");
  {
    absl::WriterMutexLock lock{&capture_event_buffer_mutex_};
    capture_event_buffer_ = capture_event_buffer;
  }
  {
    absl::MutexLock lock{&service_state_mutex_};
    service_state_.capture_status = CaptureStatus::kCaptureStarted;
  }
}

void ProducerSideServiceImpl::OnCaptureStopRequested() {
  LOG("About to send StopCaptureCommand to CaptureEventProducers (if any)");
  {
    absl::MutexLock lock{&service_state_mutex_};
    service_state_.capture_status = CaptureStatus::kCaptureStopping;

    // Wait (for a limited amount of time) for all producers to send AllEventsSent or to disconnect.
    service_state_mutex_.AwaitWithTimeout(absl::Condition(
                                              +[](ServiceState* service_state) {
                                                return service_state->producers_remaining == 0 ||
                                                       service_state->exit_requested;
                                              },
                                              &service_state_),
                                          max_wait_for_all_events_sent_);
    CHECK(service_state_.producers_remaining >= 0);
    if (service_state_.producers_remaining == 0) {
      LOG("All CaptureEventProducers have finished sending their CaptureEvents");
    } else {
      ERROR(
          "Stopped receiving CaptureEvents from CaptureEventProducers "
          "even if not all have sent all their CaptureEvents");
    }
    service_state_.capture_status = CaptureStatus::kCaptureFinished;
    service_state_.producers_remaining = 0;
  }

  {
    absl::WriterMutexLock lock{&capture_event_buffer_mutex_};
    capture_event_buffer_ = nullptr;
  }
}

void ProducerSideServiceImpl::OnExitRequest() {
  {
    absl::MutexLock lock{&service_state_mutex_};
    service_state_.exit_requested = true;
  }

  LOG("Attempting to disconnect from CaptureEventProducers as exit was requested");
  {
    absl::MutexLock lock{&server_contexts_mutex_};
    for (grpc::ServerContext* context : server_contexts_) {
      // This should cause blocking Reads on ServerReaderWriter to fail immediately.
      context->TryCancel();
    }
  }

  {
    absl::WriterMutexLock lock{&capture_event_buffer_mutex_};
    capture_event_buffer_ = nullptr;
  }
}

grpc::Status ProducerSideServiceImpl::ReceiveCommandsAndSendEvents(
    ::grpc::ServerContext* context,
    ::grpc::ServerReaderWriter< ::orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                                ::orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream) {
  LOG("A CaptureEventProducer has connected calling ReceiveCommandsAndSendEvents");

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
  // receiving CaptureEvents and AllEventsSent messages.
  std::thread receive_events_thread{&ProducerSideServiceImpl::ReceiveEventsThread, this, context,
                                    stream, &all_events_sent_received};
  receive_events_thread.join();

  // When receive_events_thread exits because stream->Read(&request) fails,
  // it means that the producer has disconnected: ask send_commands_thread to exit, too.
  receive_events_thread_exited = true;
  send_commands_thread.join();

  {
    absl::MutexLock lock{&server_contexts_mutex_};
    server_contexts_.erase(context);
  }

  LOG("Finished handling ReceiveCommandsAndSendEvents for a CaptureEventProducer");
  return grpc::Status::OK;
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

      // service_state_.capture_status has changed compared to prev_capture_status:
      // handle the change.
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
      prev_capture_status = service_state_.capture_status;
    }  // absl::MutexLock lock{&service_state_mutex_}

    // prev_capture_status has now been updated to the new service_state_.capture_status.
    switch (prev_capture_status) {
      case CaptureStatus::kCaptureStarted: {
        orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
        command.mutable_start_capture_command();
        if (!stream->Write(command)) {
          ERROR("Sending StartCaptureCommand to CaptureEventProducer");
          LOG("Terminating call to ReceiveCommandsAndSendEvents as Write failed");
          // Cause Read in ReceiveEventsThread to also fail if for some reason it hasn't already.
          context->TryCancel();
          return;
        }
        LOG("Sent StartCaptureCommand to CaptureEventProducer");
      } break;

      case CaptureStatus::kCaptureStopping: {
        orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse command;
        command.mutable_stop_capture_command();
        if (!stream->Write(command)) {
          ERROR("Sending StopCaptureCommand to CaptureEventProducer");
          LOG("Terminating call to ReceiveCommandsAndSendEvents as Write failed");
          // Cause Read in ReceiveEventsThread to also fail if for some reason it hasn't already.
          context->TryCancel();
          return;
        }
        LOG("Sent StopCaptureCommand to CaptureEventProducer");
      } break;

      case CaptureStatus::kCaptureFinished:
        break;
    }
  }
}

void ProducerSideServiceImpl::ReceiveEventsThread(
    grpc::ServerContext* /*context*/,
    grpc::ServerReaderWriter<orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse,
                             orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest>* stream,
    bool* all_events_sent_received) {
  orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest request;
  while (stream->Read(&request)) {
    {
      absl::MutexLock lock{&service_state_mutex_};
      if (service_state_.exit_requested) {
        break;
      }
    }

    switch (request.event_case()) {
      case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kBufferedCaptureEvents: {
        absl::MutexLock lock{&capture_event_buffer_mutex_};
        // capture_event_buffer_ can be nullptr if a producer sends events while not capturing.
        // Don't log an error in such a case as it could easily spam the logs.
        if (capture_event_buffer_ != nullptr) {
          for (orbit_grpc_protos::CaptureEvent event :
               request.buffered_capture_events().capture_events()) {
            capture_event_buffer_->AddEvent(std::move(event));
          }
        }
      } break;

      case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kAllEventsSent: {
        LOG("Received AllEventsSent from CaptureEventProducer");
        absl::MutexLock lock{&service_state_mutex_};
        switch (service_state_.capture_status) {
          case CaptureStatus::kCaptureStarted: {
            ERROR("CaptureEventProducer sent AllEventsSent while still capturing");
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
            ERROR("CaptureEventProducer sent AllEventsSent after the capture had finished");
          } break;
        }
      } break;

      case orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::EVENT_NOT_SET: {
        ERROR("CaptureEventProducer sent EVENT_NOT_SET");
      } break;
    }
  }

  ERROR("Receiving ReceiveCommandsAndSendEventsRequest from CaptureEventProducer");
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

}  // namespace orbit_service
