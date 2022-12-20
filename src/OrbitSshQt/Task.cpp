// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSshQt/Task.h"

#include <absl/base/attributes.h>

#include <QTimer>
#include <algorithm>
#include <chrono>
#include <limits>
#include <utility>
#include <vector>

#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitSsh/Error.h"
#include "OrbitSshQt/Error.h"

// Maximum time that the shutdown is allowed to take.
constexpr const std::chrono::milliseconds kShutdownTimeoutMs{2000};

namespace orbit_ssh_qt {

Task::Task(Session* session, std::string command)
    : session_(session), command_(std::move(command)) {
  about_to_shutdown_connection_.emplace(
      QObject::connect(session_, &Session::aboutToShutdown, this, &Task::HandleSessionShutdown));
}

orbit_base::Future<ErrorMessageOr<void>> Task::Start() {
  if (state_ == State::kInitialized) {
    SetState(State::kNoChannel);
    OnEvent();
  }

  return GetStartedFuture();
}

orbit_base::Future<ErrorMessageOr<Task::ExitCode>> Task::Stop() {
  QTimer::singleShot(kShutdownTimeoutMs, this, [this]() {
    if (state_ < State::kChannelClosed) {
      ORBIT_ERROR("Task shutdown timed out");
      SetError(Error::kOrbitServiceShutdownTimedout);
    }
  });

  if (state_ == State::kCommandRunning) {
    SetState(State::kSignalEOF);
  }
  OnEvent();

  // Since we control on which thread the stopped_future completes, we know we won't have a race
  // condition and can use the ImmediateExecutor.
  orbit_base::ImmediateExecutor executor{};
  return GetStoppedFuture().ThenIfSuccess(&executor, [this]() { return exit_code_.value(); });
}

std::string Task::ReadStdOut() {
  auto tmp = std::move(read_std_out_buffer_);
  read_std_out_buffer_ = std::string{};
  return tmp;
}

orbit_base::Future<ErrorMessageOr<Task::ExitCode>> Task::Execute() {
  // This first calls Start(). When that succeeded it calls Stop().

  // Since we control on which thread the started_futures completes, it's fine to use the
  // ImmediateExecutor here. No race condition can occur.
  orbit_base::ImmediateExecutor executor{};
  return orbit_base::UnwrapFuture(Start().ThenIfSuccess(&executor, [this]() { return Stop(); }));
}

std::string Task::ReadStdErr() {
  auto tmp = std::move(read_std_err_buffer_);
  read_std_err_buffer_ = std::string{};
  return tmp;
}

orbit_base::Future<ErrorMessageOr<void>> Task::Write(std::string_view data) {
  write_buffer_.append(data);
  WritePromise& new_entry =
      write_promises_.emplace_back(bytes_written_counter_ + write_buffer_.size());
  orbit_base::Future<ErrorMessageOr<void>> future = new_entry.promise.GetFuture();

  OnEvent();
  return future;
}

outcome::result<void> Task::run() {
  do {
    // read stdout
    bool added_new_data_to_read_buffer = false;
    while (true) {
      constexpr size_t kChunkSize = 8192;
      auto result = channel_->ReadStdOut(kChunkSize);

      if (!result && !orbit_ssh::ShouldITryAgain(result)) {
        if (added_new_data_to_read_buffer) {
          emit readyReadStdOut();
        }
        return result.error();
      }
      if (!result) {
        if (added_new_data_to_read_buffer) {
          emit readyReadStdOut();
        }
        break;
      }
      if (result.value().empty()) {
        // Channel closed
        if (added_new_data_to_read_buffer) {
          emit readyReadStdOut();
        }
        SetState(State::kWaitChannelClosed);
        break;
      }

      read_std_out_buffer_.append(std::move(result).value());
      added_new_data_to_read_buffer = true;
    }

    // read stderr
    added_new_data_to_read_buffer = false;
    while (true) {
      constexpr size_t kChunkSize = 8192;
      auto result = channel_->ReadStdErr(kChunkSize);

      if (!result && !orbit_ssh::ShouldITryAgain(result)) {
        if (added_new_data_to_read_buffer) {
          emit readyReadStdErr();
        }
        return result.error();
      }
      if (!result) {
        if (added_new_data_to_read_buffer) {
          emit readyReadStdErr();
        }
        break;
      }
      if (result.value().empty()) {
        // Channel closed
        if (added_new_data_to_read_buffer) {
          emit readyReadStdErr();
        }
        SetState(State::kWaitChannelClosed);
        break;
      }
      read_std_err_buffer_.append(std::move(result).value());
      added_new_data_to_read_buffer = true;
    }

    // If the state here is kWaitChannelClosed, that means a close from the remote side was
    // detected. This means writing is not possible/necessary anymore, therefore: return early.
    if (CurrentState() == State::kWaitChannelClosed) {
      return outcome::success();
    }

    // write
    if (!write_buffer_.empty()) {
      OUTCOME_TRY(auto&& result, channel_->Write(write_buffer_));
      write_buffer_ = write_buffer_.substr(result);
      emit bytesWritten(result);
      bytes_written_counter_ += result;
      while (!write_promises_.empty() &&
             write_promises_.front().completes_when_bytes_written <= bytes_written_counter_) {
        write_promises_.front().promise.SetResult(outcome::success());
        write_promises_.pop_front();
      }

      // We set the counter to 0 and adjust the thresholds to avoid overflows once in a while.
      if (bytes_written_counter_ > std::numeric_limits<uint64_t>::max() / 2) {
        for (auto& write_promise : write_promises_) {
          write_promise.completes_when_bytes_written -= bytes_written_counter_;
        }
        bytes_written_counter_ = 0;
      }
    }

    // Channel::ReadStdout, Channel::ReadStderr, and Channel::Write all potentially process packets
    // coming from the TCP socket. That means after handling stderr there could already be new data
    // in the queue for stdout. So we have to keep reading until no more data is available in any of
    // the streams.
  } while (channel_->GetNumBytesToRead() > 0);

  return outcome::success();
}

outcome::result<void> Task::startup() {
  if (!data_event_connection_) {
    data_event_connection_.emplace(
        QObject::connect(session_, &Session::dataEvent, this, &Task::OnEvent));
  }

  switch (CurrentState()) {
    case State::kInitialized:
    case State::kNoChannel: {
      auto* session = session_->GetRawSession();
      if (session == nullptr) {
        return orbit_ssh::Error::kEagain;
      }

      OUTCOME_TRY(auto&& channel, orbit_ssh::Channel::OpenChannel(session_->GetRawSession()));
      channel_ = std::move(channel);
      SetState(State::kChannelInitialized);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kChannelInitialized: {
      OUTCOME_TRY(channel_->Exec(command_));
      SetState(State::kCommandRunning);
      break;
    }
    case State::kStarted:
    case State::kCommandRunning:
    case State::kStopping:
    case State::kSignalEOF:
    case State::kWaitRemoteEOF:
    case State::kSignalChannelClose:
    case State::kWaitChannelClosed:
    case State::kStopped:
    case State::kChannelClosed:
    case State::kError:
      ORBIT_UNREACHABLE();
  }

  return outcome::success();
}

outcome::result<void> Task::shutdown() {
  switch (state_) {
    case State::kInitialized:
    case State::kNoChannel:
    case State::kChannelInitialized:
    case State::kStarted:
    case State::kCommandRunning:
      ORBIT_UNREACHABLE();

    case State::kStopping:
    case State::kSignalEOF: {
      OUTCOME_TRY(channel_->SendEOF());
      SetState(State::kWaitRemoteEOF);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kWaitRemoteEOF: {
      OUTCOME_TRY(channel_->WaitRemoteEOF());
      SetState(State::kSignalChannelClose);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kSignalChannelClose: {
      OUTCOME_TRY(channel_->Close());
      SetState(State::kWaitChannelClosed);
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kWaitChannelClosed: {
      OUTCOME_TRY(channel_->WaitClosed());
      SetState(State::kChannelClosed);
      // The exit status is only guaranteed to be available after the channel is really closed on
      // both sides
      exit_code_ = ExitCode{channel_->GetExitStatus()};
      emit finished(*exit_code_.value());
      ABSL_FALLTHROUGH_INTENDED;
    }
    case State::kStopped:
    case State::kChannelClosed:
      data_event_connection_ = std::nullopt;
      about_to_shutdown_connection_ = std::nullopt;
      channel_ = std::nullopt;
      break;
    case State::kError:
      ORBIT_UNREACHABLE();
  }

  return outcome::success();
}

void Task::SetError(std::error_code e) {
  data_event_connection_ = std::nullopt;
  about_to_shutdown_connection_ = std::nullopt;
  StateMachineHelper::SetError(e);
  channel_ = std::nullopt;

  while (!write_promises_.empty()) {
    write_promises_.front().promise.SetResult(ErrorMessage{e.message()});
    write_promises_.pop_front();
  }
}

void Task::HandleSessionShutdown() {
  if (CurrentState() != State::kNoChannel && CurrentState() != State::kError) {
    SetError(Error::kUncleanSessionShutdown);
  }

  session_ = nullptr;
}

void Task::HandleEagain() {
  if (session_ != nullptr) {
    session_->HandleEagain();
  }
}

}  // namespace orbit_ssh_qt
