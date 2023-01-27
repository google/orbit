// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SshQtTestUtils/KillProcessListeningOnTcpPort.h"

#include <absl/strings/str_format.h>

#include <variant>
#include <vector>

#include "OrbitBase/Result.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/Task.h"
#include "QtTestUtils/WaitForWithTimeout.h"

namespace orbit_ssh_qt_test_utils {
using orbit_qt_test_utils::ConsiderTimeoutAnError;
using orbit_qt_test_utils::WaitForWithTimeout;
using orbit_ssh_qt::Task;

ErrorMessageOr<void> KillProcessListeningOnTcpPort(orbit_ssh_qt::Session* session, int tcp_port) {
  Task kill_task{session, absl::StrFormat("kill $(fuser %d/tcp)", tcp_port)};
  OUTCOME_TRY(Task::ExitCode exit_code,
              ConsiderTimeoutAnError(WaitForWithTimeout(kill_task.Execute())));

  if (*exit_code != 0) {
    return ErrorMessage{
        absl::StrFormat("The `kill` command returned a non-zero exit code: %d", *exit_code)};
  }

  return outcome::success();
}

}  // namespace orbit_ssh_qt_test_utils
