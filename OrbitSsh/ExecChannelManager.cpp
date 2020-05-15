// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/ExecChannelManager.h"

#include <memory>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/ResultType.h"

namespace OrbitSsh {

ExecChannelManager::ExecChannelManager(
    Session* session_ptr, std::string command,
    std::function<void(std::string)> output_callback,
    std::function<void(int)> exit_callback)
    : session_ptr_(session_ptr),
      command_(command),
      output_callback_(output_callback),
      exit_callback_(exit_callback) {
  FAIL_IF(!output_callback_, "No valid output callback provided");
  FAIL_IF(!exit_callback_, "No valid exit status callback provided");
}

ExecChannelManager::~ExecChannelManager() {
  if (state_ == State::kRunning) {
    ERROR("Exec Channel still running when destroyed");
  }
}

ResultType ExecChannelManager::Read(std::string* text) {
  *text = "";
  ResultType result = channel_->Read(text);

  if (result != ResultType::kSuccess) return result;

  if (text->empty()) return ResultType::kAgain;

  return ResultType::kSuccess;
}

// Tick progresses the state when appropriate and is responsible for calling
// output_callback and exit_callback. This function should be called
// periodically.
ExecChannelManager::State ExecChannelManager::Tick() {
  ResultType result = ResultType::kAgain;

  switch (state_) {
    case State::kNotInitialized: {
      result = Channel::CreateOpenSession(session_ptr_, &channel_);
      if (result == ResultType::kSuccess) state_ = State::kChannelOpened;
      break;
    }
    case State::kChannelOpened: {
      result = channel_->Exec(command_);
      if (result == ResultType::kSuccess) state_ = State::kRunning;
      break;
    }
    case State::kRunning: {
      std::string text;
      result = Read(&text);
      if (result == ResultType::kSuccess) {
        output_callback_(text);
      }
      if (channel_->GetRemoteEOF()) {
        exit_callback_(channel_->GetExitStatus());
        state_ = State::kFinished;
      }
      break;
    }
    case State::kFinished:
      break;
    case State::kFailed:
      break;
  }

  if (result == ResultType::kError) {
    int exit_status =
        channel_->GetExitStatus() != 0 ? channel_->GetExitStatus() : -1;
    ERROR("Exec Channel failed with exit status %d", exit_status);
    exit_callback_(exit_status);
    state_ = State::kFailed;
  }

  return state_;
}

}  // namespace OrbitSsh