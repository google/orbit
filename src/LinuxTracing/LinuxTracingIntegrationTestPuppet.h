// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_
#define LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_

namespace orbit_linux_tracing {

struct LinuxTracingIntegrationTestPuppetConstants {
  LinuxTracingIntegrationTestPuppetConstants() = delete;

  constexpr static const char* kSharedObjectFileName =
      "libLinuxTracingIntegrationTestPuppetSharedObject.so";

  constexpr static const char* kDlopenCommand = "dlopen";

  constexpr static const char* kDoneResponse = "DONE";
};

int LinuxTracingIntegrationTestPuppetMain();

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_
