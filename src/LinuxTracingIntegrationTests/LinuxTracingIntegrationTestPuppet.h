// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_
#define LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_

#include <stdint.h>

#include <array>

namespace orbit_linux_tracing_integration_tests {

struct LinuxTracingIntegrationTestPuppetConstants {
  LinuxTracingIntegrationTestPuppetConstants() = delete;

  constexpr static uint64_t kSleepCount = 1000;

  constexpr static uint64_t kOuterFunctionCallCount = 2;
  constexpr static uint64_t kOuterFunctionReturnValue = 0x0123456789ABCDEF;
  constexpr static const char* kOuterFunctionName = "OuterFunctionToInstrument";
  constexpr static uint64_t kInnerFunctionCallCount = 3;
  constexpr static std::array<uint64_t, 6> kInnerFunctionCallArgs = {1, 2, 3, 4, 5, 6};
  constexpr static const char* kInnerFunctionName = "InnerFunctionToInstrument";

  constexpr static const char* kNewThreadName = "Thread Name";

  constexpr static const char* kSharedObjectFileName =
      "libLinuxTracingIntegrationTestPuppetSharedObject.so";

  constexpr static uint64_t kFrameCount = 60;

  constexpr static const char* kSleepCommand = "sleep";
  constexpr static const char* kCallOuterFunctionCommand = "CallOuterFunction";
  constexpr static const char* kPthreadSetnameNpCommand = "pthread_setname_np";
  constexpr static const char* kDlopenCommand = "dlopen";
  constexpr static const char* kVulkanTutorialCommand = "VulkanTutorial";

  constexpr static const char* kDoneResponse = "DONE";
};

int LinuxTracingIntegrationTestPuppetMain();

}  // namespace orbit_linux_tracing_integration_tests

#endif  // LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_
