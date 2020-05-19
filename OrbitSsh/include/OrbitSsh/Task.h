// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_TASKfine ORBIT_SSH_TASK_H_

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "Channel.h"

namespace OrbitSsh {

// Task manages a ssh exec channel. An exec channel executes one
// command on the remote server, periodically returns its output and returns the
// exit code when the command returns. The Tick function should be called
// periodically until the command finishes to receive the commands output.
// The class is constructed with the command to be executed and three callbacks.
// std_out_callback is called whenever the command produces stdout output,
// std_err_callback whenever it produces stderr output and exit_callback is
// called once when either the command exits on its own.
template <typename ResultType>
class Task {
 public:
  using StringCallback =
      std::function<void(std::string, std::optional<ResultType>*)>;
  using ExitCallback =
      std::function<ResultType(int, std::optional<ResultType>*)>;
  Task(Session* session_ptr, std::string command,
       StringCallback std_out_callback, StringCallback std_err_callback,
       ExitCallback exit_callback);

  outcome::result<ResultType> Run();

 private:
  enum class State { kNotInitialized, kChannelOpened, kRunning, kFinished };

  outcome::result<void> Read();
  std::optional<ResultType> result_ = std::nullopt;
  Session* session_ptr_;
  std::optional<Channel> channel_;
  std::string command_;
  std::function<void(std::string, std::optional<ResultType>*)>
      std_out_callback_;
  std::function<void(std::string, std::optional<ResultType>*)>
      std_err_callback_;
  std::function<ResultType(int, std::optional<ResultType>*)> exit_callback_;
  State state_ = State::kNotInitialized;
};

// template implementation
template <typename ResultType>
Task<ResultType>::Task(Session* session_ptr, std::string command,
                       StringCallback std_out_callback,
                       StringCallback std_err_callback,
                       ExitCallback exit_callback)
    : session_ptr_(session_ptr),
      command_(command),
      std_out_callback_(std_out_callback),
      std_err_callback_(std_err_callback),
      exit_callback_(exit_callback) {
  FAIL_IF(!exit_callback_, "Did not provide valid exit callback");
}

template <typename ResultType>
outcome::result<void> Task<ResultType>::Read() {
  if (std_out_callback_) {
    auto std_out = channel_->ReadStdOut();

    // if std out reading has error -> return it
    if (!std_out && !shouldITryAgain(std_out)) return std_out.error();

    // if std out not empty -> std out callback
    if (std_out && !std_out.value().empty()) {
      std_out_callback_(std::move(std_out.value()), &result_);
    }
  }

  if (std_err_callback_) {
    auto std_err = channel_->ReadStdErr();

    // if std err reading has error -> return it
    if (!std_err && !shouldITryAgain(std_err)) return std_err.error();

    // if std err not empty -> std err callback
    if (std_err && !std_err.value().empty()) {
      std_err_callback_(std::move(std_err.value()), &result_);
    }
  }

  return outcome::success();
}

// Tick progresses the state when appropriate and is responsible for calling
// std_out_callback and exit_callback. This function should be called
// periodically.
template <typename ResultType>
outcome::result<ResultType> Task<ResultType>::Run() {
  switch (state_) {
    case State::kNotInitialized: {
      OUTCOME_TRY(channel, Channel::OpenChannel(session_ptr_));
      channel_ = std::move(channel);
      state_ = State::kChannelOpened;
    }
    case State::kChannelOpened: {
      OUTCOME_TRY(channel_->Exec(command_));
      state_ = State::kRunning;
    }
    case State::kRunning: {
      OUTCOME_TRY(Read());

      // when remote did not sent EOF -> there will be more data in the future
      if (!channel_->GetRemoteEOF()) return Error::kEagain;

      state_ = State::kFinished;
      return outcome::success(
          exit_callback_(channel_->GetExitStatus(), &result_));
    }
    case State::kFinished:
    default:
      return Error::kTaskUsedAfterFinish;
  }
}

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_TASK_H_
