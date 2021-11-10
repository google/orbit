// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_UTILS_H_
#define LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_UTILS_H_

#include <absl/strings/match.h>
#include <unistd.h>

#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"

namespace orbit_linux_tracing_integration_tests {

[[nodiscard]] inline bool IsRunningAsRoot() { return geteuid() == 0; }

[[nodiscard]] inline bool CheckIsRunningAsRoot() {
  if (IsRunningAsRoot()) {
    return true;
  }

  ERROR("Root required for this test");
  return false;
}

}  // namespace orbit_linux_tracing_integration_tests

#endif  // LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_UTILS_H_
