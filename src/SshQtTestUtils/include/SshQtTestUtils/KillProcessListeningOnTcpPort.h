// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_TEST_UTILS_KILL_PROCESS_LISTENING_ON_TCP_PORT_H_
#define ORBIT_SSH_QT_TEST_UTILS_KILL_PROCESS_LISTENING_ON_TCP_PORT_H_

#include "OrbitBase/Result.h"
#include "OrbitSshQt/Session.h"

namespace orbit_ssh_qt_test_utils {

// Launches a task through the given SSH session that kills the process listening on the given TCP
// port. Reports an error if that fails. Note that also an error is reported if it fails to kill
// any process.
ErrorMessageOr<void> KillProcessListeningOnTcpPort(orbit_ssh_qt::Session* session, int tcp_port);

}  // namespace orbit_ssh_qt_test_utils

#endif  // ORBIT_SSH_QT_TEST_UTILS_KILL_PROCESS_LISTENING_ON_TCP_PORT_H_
