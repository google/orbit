// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/SshManager.h"

#include <libssh2.h>

#include <memory>
#include <queue>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Credentials.h"
#include "OrbitSsh/ExecChannelManager.h"
#include "OrbitSsh/SessionManager.h"
#include "OrbitSsh/TunnelManager.h"

namespace OrbitSsh {

SshManager::SshManager(Credentials credentials, std::queue<Task> pre_tasks,
                       Task main_task, std::vector<int> tunnel_ports)
    : session_manager_(credentials),
      pre_tasks_(pre_tasks),
      main_task_(main_task),
      tunnel_ports_(tunnel_ports) {
  FAIL_IF(libssh2_init(0) != 0, "Unable to initialize libssh2");
  LOG("Ssh initialized");
}

SshManager::~SshManager() noexcept { /*libssh2_exit();*/ }

// Tick progresses through different states until it reaches
// kMainAndTunnelRunning, in which it will keep the main task and tunnels alive
// until its closed
// 1. Setup and initialization process of a session. This is managed by
// SessionManager and progresses with calling its Tick function. Once the
// Session reaches state authenticated, it is running.
// 2. Execution of pre tasks. Each task is managed by a new instance of
// ExecChannelManager and progresses with calling its Tick function. When a task
// is finished or fails, the callback PreTaskExit is called, which will set the
// state so the next pre task is started. Once all pre task are done, the main
// task is started
// 3. Start of main task execution. A new ExecChannelManager handles the
// execution of the main task. Once the main task is running, the ssh tunnels
// are started.
// 4. Running of Main task and Tunnels. The main task progress is managed by
// exec_channel_ by calling its Tick function. Each tunnel is managed by one
// TunnelManager, which packages through the tunnel everytime it Tick is called
outcome::result<void> SshManager::Tick() {
  switch (state_) {
    case State::kNotInitialized: {
      OUTCOME_TRY(session_manager_.Initialize());
      state_ = State::kPreTasksRunning;
    }
    case State::kSessionRunning:
    case State::kPreTasksRunning: {
      if (!exec_channel_ && !pre_tasks_.empty()) {
        Task pre_task = pre_tasks_.front();
        pre_tasks_.pop();
        exec_channel_ = ExecChannelManager(
            session_manager_.GetSessionPtr(), pre_task.command,
            [pre_task](std::string text) { pre_task.output_callback(text); },
            [pre_task, this](int exit_code) {
              pre_task.exit_callback(exit_code);
              state_ = State::kSessionRunning;
            });
      }
      if (exec_channel_) {
        OUTCOME_TRY(exec_channel_->Run(SuccessWhen::kFinished));
        exec_channel_ = std::nullopt;
      }
      state_ = State::kMainTaskStarting;
    }
    case State::kMainTaskStarting: {
      if (!exec_channel_) {
        exec_channel_ = ExecChannelManager(
            session_manager_.GetSessionPtr(), main_task_.command,
            [this](std::string text) { main_task_.output_callback(text); },
            [this](int exit_code) { this->MainTaskExit(exit_code); });
      }

      OUTCOME_TRY(exec_channel_->Run(SuccessWhen::kRunning));
      StartPortForwarding();
      state_ = State::kMainAndTunnelsRunning;
    }
    case State::kMainAndTunnelsRunning: {
      const auto result = exec_channel_->Run(SuccessWhen::kFinished);

      if (!result && !shouldITryAgain(result)) {
        return result.error();
      }

      for (TunnelManager& tunnel : tunnels_) {
        const auto result = tunnel.Tick();
        if (!result && !shouldITryAgain(result)) {
          return result.error();
        }
      }
      break;
    }
    case State::kClosed: {
      break;
    }
  }
  return outcome::success();
}

// This closes all members in the correct order, depending on the state its in.
// In the case this function returns kAgain, it needs to be called again to
// continue the closing. This can happen because some of the closing of the
// members might return kAgain themselves
outcome::result<void> SshManager::Close() {
  switch (state_) {
    case State::kMainAndTunnelsRunning: {
      OUTCOME_TRY(CloseTunnels());
      state_ = State::kSessionRunning;
    }

    case State::kSessionRunning: {
      OUTCOME_TRY(session_manager_.Close());
      state_ = State::kNotInitialized;
    }

    // running exec tasks can't be cancelled
    case State::kPreTasksRunning:
    case State::kMainTaskStarting:

    // do nothing
    case State::kNotInitialized:
    case State::kClosed:
      break;
  }

  state_ = State::kClosed;
  return outcome::success();
}

void SshManager::MainTaskExit(int exit_code) {
  main_task_.exit_callback(exit_code);
  ERROR("Main task finished with exit code %d", exit_code);
  // TODO handle this case.
  // If this happens when it was not intended, it likely means the session
  // disconnected
}

void SshManager::StartPortForwarding() {
  if (tunnels_.empty()) {
    for (auto port : tunnel_ports_) {
      tunnels_.emplace_back(session_manager_.GetSessionPtr(), port, port);
    }
  }
}

outcome::result<void> SshManager::CloseTunnels() {
  for (TunnelManager& tunnel : tunnels_) {
    OUTCOME_TRY(tunnel.Close());
  }

  return outcome::success();
}

}  // namespace OrbitSsh
