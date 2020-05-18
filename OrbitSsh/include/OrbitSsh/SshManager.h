// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SSH_HANDLER_H_
#define ORBIT_SSH_SSH_HANDLER_H_

#include <filesystem>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "OrbitSsh/Context.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSsh/ExecChannelManager.h"
#include "OrbitSsh/SessionManager.h"
#include "OrbitSsh/TunnelManager.h"

namespace OrbitSsh {

// The SshManager class abstracts all ssh capabilities Orbit uses. It needs to
// be constructed with the ssh credentials, tasks that need to be done before
// the main task (OrbitService) and ports that should be forwarded. Task here
// refers to command line commands. Then the Tick() function needs to be called
// periodically.
class SshManager {
  enum class State {
    kNotInitialized,
    kSessionRunning,
    kPreTasksRunning,
    kMainTaskStarting,
    kMainAndTunnelsRunning,
    kClosed,
  };

 public:
  // Task refers to a shell command.
  // * command is the shell command that will be executed.
  // * output_callback will be called everytime the task produces stdout output
  // * exit_callback will be called once with the exit code, when the task
  // returns
  struct Task {
    std::string command;
    std::function<void(std::string)> output_callback;
    std::function<void(int)> exit_callback;
  };
  SshManager(Context* context, Credentials credentials,
             std::queue<Task> pre_tasks, Task main_task,
             std::vector<int> tunnel_ports);

  outcome::result<void> Tick();
  outcome::result<void> Close();

 private:
  void MainTaskExit(int exit_code);
  void StartPortForwarding();
  outcome::result<void> CloseTunnels();
  State state_ = State::kNotInitialized;
  SessionManager session_manager_;
  std::optional<ExecChannelManager> exec_channel_;
  std::vector<TunnelManager> tunnels_;
  std::queue<Task> pre_tasks_;
  Task main_task_;
  std::vector<int> tunnel_ports_;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SSH_HANDLER_H_
