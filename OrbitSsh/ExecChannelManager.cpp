// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/ExecChannelManager.h"

#include <memory>

#include "OrbitBase/Logging.h"

namespace OrbitSsh {

ExecChannelManager::ExecChannelManager(
    Session* session_ptr, std::string command,
    std::function<void(std::string)> output_callback,
    std::function<void(int)> exit_callback)
    : session_ptr_(session_ptr),
      command_(command),
      output_callback_(output_callback),
      exit_callback_(exit_callback) {}

// Tick progresses the state when appropriate and is responsible for calling
// output_callback and exit_callback. This function should be called
// periodically.
outcome::result<void> ExecChannelManager::Run(SuccessWhen successWhen) {
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
      if (successWhen == SuccessWhen::kRunning) {
        return outcome::success();
      }
      OUTCOME_TRY(data, channel_->Read());
      if (output_callback_) {
        output_callback_(std::move(data));
      }

      const auto result = channel_->GetRemoteEOF();
      if (result) {
        const auto exit_status = channel_->GetExitStatus();

        if (exit_callback_) {
          exit_callback_(exit_status);
        }

        state_ = State::kFinished;
        return outcome::success();
      } else {
        return Error::kEagain;
      }
    }
    case State::kFinished:
      break;
    case State::kFailed:
      break;
  }

  return outcome::success();
}

}  // namespace OrbitSsh
