// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureEventProducer/CaptureEventProducer.h"

#include <absl/synchronization/mutex.h>
#include <absl/time/time.h>

#include <chrono>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"

using orbit_grpc_protos::ProducerSideService;
using orbit_grpc_protos::ReceiveCommandsAndSendEventsResponse;

namespace orbit_capture_event_producer {

void CaptureEventProducer::BuildAndStart(const std::shared_ptr<grpc::Channel>& channel) {
  ORBIT_CHECK(channel != nullptr);

  producer_side_service_stub_ = ProducerSideService::NewStub(channel);
  ORBIT_CHECK(producer_side_service_stub_ != nullptr);

  connect_and_receive_commands_thread_ = std::thread{[this] { ConnectAndReceiveCommandsThread(); }};
}

void CaptureEventProducer::ShutdownAndWait() {
  {
    absl::WriterMutexLock lock{&shutdown_requested_mutex_};
    ORBIT_CHECK(!shutdown_requested_);
    shutdown_requested_ = true;
  }

  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    if (context_ != nullptr) {
      ORBIT_LOG("Attempting to disconnect from ProducerSideService as exit was requested");
      context_->TryCancel();
    }
  }

  ORBIT_CHECK(connect_and_receive_commands_thread_.joinable());
  connect_and_receive_commands_thread_.join();

  producer_side_service_stub_.reset();
  // If producer_side_service_stub_ held the last reference to a gRPC object,
  // the internal grpc_shutdown will be executed. This can use a detached thread.
  // From the docs of grpc_shutdown: "The last call to grpc_shutdown will initiate
  // cleaning up of grpc library internals, which can happen in another thread".
  // Give that a moment to complete, as otherwise that could lead to a SIGSEGV on exit.
  std::this_thread::sleep_for(std::chrono::milliseconds{1});
}

bool CaptureEventProducer::SendCaptureEvents(
    const orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest& send_events_request) {
  ORBIT_CHECK(send_events_request.event_case() ==
              orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest::kBufferedCaptureEvents);

  ORBIT_CHECK(producer_side_service_stub_ != nullptr);
  {
    // Acquiring the mutex just for the CHECK might seem expensive,
    // but the gRPC call that follows is orders of magnitude slower.
    absl::ReaderMutexLock lock{&shutdown_requested_mutex_};
    ORBIT_CHECK(!shutdown_requested_);
  }

  bool write_succeeded;
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    if (stream_ == nullptr) {
      ORBIT_ERROR("Sending BufferedCaptureEvents to ProducerSideService: not connected");
      return false;
    }
    write_succeeded = stream_->Write(send_events_request);
  }
  if (!write_succeeded) {
    ORBIT_ERROR("Sending BufferedCaptureEvents to ProducerSideService");
  }
  return write_succeeded;
}

bool CaptureEventProducer::NotifyAllEventsSent() {
  ORBIT_CHECK(producer_side_service_stub_ != nullptr);
  {
    absl::ReaderMutexLock lock{&shutdown_requested_mutex_};
    ORBIT_CHECK(!shutdown_requested_);
  }

  orbit_grpc_protos::ReceiveCommandsAndSendEventsRequest all_events_sent_request;
  all_events_sent_request.mutable_all_events_sent();
  bool write_succeeded;
  {
    absl::ReaderMutexLock lock{&context_and_stream_mutex_};
    if (stream_ == nullptr) {
      ORBIT_ERROR("Sending AllEventsSent to ProducerSideService: not connected");
      return false;
    }
    write_succeeded = stream_->Write(all_events_sent_request);
  }
  if (write_succeeded) {
    ORBIT_LOG("Sent AllEventsSent to ProducerSideService");
  } else {
    ORBIT_ERROR("Sending AllEventsSent to ProducerSideService");
  }
  return write_succeeded;
}

void CaptureEventProducer::ConnectAndReceiveCommandsThread() {
  ORBIT_CHECK(producer_side_service_stub_ != nullptr);
  orbit_base::SetCurrentThreadName("ConnectRcvCmds");

  while (true) {
    {
      absl::ReaderMutexLock lock{&shutdown_requested_mutex_};
      if (shutdown_requested_) {
        break;
      }
    }

    // Attempt to connect to ProducerSideService. Note that getting a stream_ != nullptr doesn't
    // mean that the service is listening nor that the connection is actually established.
    {
      absl::WriterMutexLock lock{&context_and_stream_mutex_};
      context_ = std::make_unique<grpc::ClientContext>();
      stream_ = producer_side_service_stub_->ReceiveCommandsAndSendEvents(context_.get());
    }

    if (stream_ == nullptr) {
      ORBIT_ERROR(
          "Calling ReceiveCommandsAndSendEvents to establish "
          "gRPC connection with ProducerSideService");
      // This is the reason why we protect shutdown_requested_ with an absl::Mutex instead
      // of using an std::atomic<bool>: so that we can use (Reader)LockWhenWithTimeout
      // to wait for reconnection_delay_ms_ or until shutdown_requested_ has become true.
      shutdown_requested_mutex_.ReaderLockWhenWithTimeout(
          absl::Condition(&shutdown_requested_),
          absl::Milliseconds(static_cast<int64_t>(reconnection_delay_ms_)));
      shutdown_requested_mutex_.ReaderUnlock();
      continue;
    }
    ORBIT_LOG("Called ReceiveCommandsAndSendEvents on ProducerSideService");

    while (true) {
      ReceiveCommandsAndSendEventsResponse response;
      bool read_succeeded;
      {
        absl::ReaderMutexLock lock{&context_and_stream_mutex_};
        read_succeeded = stream_->Read(&response);
      }
      if (!read_succeeded) {
        ORBIT_ERROR("Receiving ReceiveCommandsAndSendEventsResponse from ProducerSideService");
        if (last_command_ == ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand) {
          last_command_ = ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand;
          OnCaptureStop();
          last_command_ = ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand;
          OnCaptureFinished();
        } else if (last_command_ == ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand) {
          last_command_ = ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand;
          OnCaptureFinished();
        }
        ORBIT_LOG("Terminating call to ReceiveCommandsAndSendEvents");
        {
          absl::WriterMutexLock lock{&context_and_stream_mutex_};
          stream_->Finish().IgnoreError();
          context_ = nullptr;
          stream_ = nullptr;
        }

        // Wait to avoid continuously trying to reconnect when OrbitService is not reachable.
        shutdown_requested_mutex_.ReaderLockWhenWithTimeout(
            absl::Condition(&shutdown_requested_),
            absl::Milliseconds(static_cast<int64_t>(reconnection_delay_ms_)));
        shutdown_requested_mutex_.ReaderUnlock();
        break;
      }

      switch (response.command_case()) {
        case ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand: {
          ORBIT_LOG("ProducerSideService sent StartCaptureCommand");
          if (last_command_ == ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand) {
            last_command_ = ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand;
            OnCaptureStart(response.start_capture_command().capture_options());
          } else if (last_command_ == ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand) {
            last_command_ = ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand;
            OnCaptureFinished();
            last_command_ = ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand;
            OnCaptureStart(response.start_capture_command().capture_options());
          }
        } break;

        case ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand: {
          ORBIT_LOG("ProducerSideService sent StopCaptureCommand");
          if (last_command_ == ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand) {
            last_command_ = ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand;
            OnCaptureStop();
          } else if (last_command_ ==
                     ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand) {
            last_command_ = ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand;
            OnCaptureStart(orbit_grpc_protos::CaptureOptions{});
            last_command_ = ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand;
            OnCaptureStop();
          }
        } break;

        case ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand: {
          ORBIT_LOG("ProducerSideService sent CaptureFinishedCommand");
          if (last_command_ == ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand) {
            last_command_ = ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand;
            OnCaptureFinished();
          } else if (last_command_ == ReceiveCommandsAndSendEventsResponse::kStartCaptureCommand) {
            last_command_ = ReceiveCommandsAndSendEventsResponse::kStopCaptureCommand;
            OnCaptureStop();
            last_command_ = ReceiveCommandsAndSendEventsResponse::kCaptureFinishedCommand;
            OnCaptureFinished();
          }
        } break;

        case ReceiveCommandsAndSendEventsResponse::COMMAND_NOT_SET: {
          ORBIT_ERROR("ProducerSideService sent COMMAND_NOT_SET");
        } break;
      }
    }
  }
}

}  // namespace orbit_capture_event_producer
