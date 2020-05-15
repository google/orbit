// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_EXEC_CHANNEL_MANAGER_H_
#define ORBIT_SSH_EXEC_CHANNEL_MANAGER_H_

#include <functional>
#include <memory>
#include <string>

#include "Channel.h"
#include "ResultType.h"

namespace OrbitSsh {
// ExecChannelManager manages a ssh exec channel. An exec channel executes one
// command on the remote server, periodically returns its output and returns the
// exit code when the command returns. The Tick function should be called
// periodically until the command finishes to receive the commands output.
// The class is constructed with the command to be executed and two callbacks.
// output_callback is called whenever the command produces stdout output.
// exit_callback is called once when either the command exits on its own, or
// when an error occurred
class ExecChannelManager {
 public:
  enum class State {
    kNotInitialized,
    kChannelOpened,
    kRunning,
    kFinished,
    kFailed
  };

  ExecChannelManager(Session* session_ptr, std::string command,
                     std::function<void(std::string)> output_callback,
                     std::function<void(int)> exit_callback);
  ExecChannelManager() = delete;
  ExecChannelManager(const ExecChannelManager&) = delete;
  ExecChannelManager& operator=(const ExecChannelManager&) = delete;
  ExecChannelManager(ExecChannelManager&& other) = default;
  ExecChannelManager& operator=(ExecChannelManager&& other) = default;
  ~ExecChannelManager();

  State Tick();

 private:
  ResultType Read(std::string* text);
  Session* session_ptr_;
  std::optional<Channel> channel_;
  std::string command_;
  std::function<void(std::string)> output_callback_;
  std::function<void(int)> exit_callback_;
  State state_ = State::kNotInitialized;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_EXEC_CHANNEL_MANAGER_H_