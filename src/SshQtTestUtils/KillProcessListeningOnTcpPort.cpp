// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SshQtTestUtils/KillProcessListeningOnTcpPort.h"

#include <absl/strings/str_format.h>

#include <QList>
#include <QSignalSpy>
#include <QVariant>
#include <optional>
#include <string>

#include "OrbitBase/Result.h"
#include "OrbitSshQt/Session.h"
#include "OrbitSshQt/Task.h"
#include "QtTestUtils/WaitFor.h"

namespace orbit_ssh_qt_test_utils {

ErrorMessageOr<void> KillProcessListeningOnTcpPort(orbit_ssh_qt::Session* session, int tcp_port) {
  orbit_ssh_qt::Task kill_task{session, absl::StrFormat("kill $(fuser %d/tcp)", tcp_port)};
  QSignalSpy finished_signal{&kill_task, &orbit_ssh_qt::Task::finished};
  auto start_result = orbit_qt_test_utils::WaitFor(kill_task.Start());

  if (orbit_qt_test_utils::HasTimedOut(start_result)) {
    return ErrorMessage{"Failed to start `kill`. Start procedure timed out."};
  }
  OUTCOME_TRY(orbit_qt_test_utils::GetValue(start_result).value());

  orbit_qt_test_utils::WaitForResult<ErrorMessageOr<void>> stop_result =
      orbit_qt_test_utils::WaitFor(kill_task.Stop());

  if (orbit_qt_test_utils::HasTimedOut(stop_result)) {
    return ErrorMessage{"Failed to stop `kill`. Stop procedure timed out."};
  }

  OUTCOME_TRY(orbit_qt_test_utils::GetValue(stop_result).value());

  if (finished_signal.empty() && !finished_signal.wait()) {
    return ErrorMessage{"Waiting for finished signal timed out."};
  }

  const int exit_code = finished_signal[0][0].toInt();
  if (exit_code != 0) {
    return ErrorMessage{
        absl::StrFormat("The `kill` command returned a non-zero exit code: %d", exit_code)};
  }

  return outcome::success();
}

}  // namespace orbit_ssh_qt_test_utils
