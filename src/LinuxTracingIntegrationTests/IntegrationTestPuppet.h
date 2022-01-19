// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_
#define LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_

#include <stdint.h>

#include <array>
#include <cstdlib>

namespace orbit_linux_tracing_integration_tests {

struct IntegrationTestPuppetConstants {
  IntegrationTestPuppetConstants() = delete;

  constexpr static uint64_t kSleepCount = 1000;

  constexpr static uint64_t kOuterFunctionCallCount = 2;
  constexpr static uint64_t kOuterFunctionReturnValue = 0x0123456789ABCDEF;
  constexpr static const char* kOuterFunctionName = "OuterFunctionToInstrument";
  constexpr static uint64_t kInnerFunctionCallCount = 3;
  constexpr static std::array<uint64_t, 6> kInnerFunctionCallArgs = {1, 2, 3, 4, 5, 6};
  constexpr static const char* kInnerFunctionName = "InnerFunctionToInstrument";

  constexpr static const char* kNewThreadName = "Thread Name";

  constexpr static const char* kSharedObjectFileName = "libIntegrationTestPuppetSharedObject.so";

  constexpr static uint64_t kFrameCount = 60;

  // Large enough to be well measurable.
  constexpr static uint64_t kRssIncreaseB = 100ULL * 1024 * 1024;

  constexpr static const char* kUseOrbitApiFunctionName = "UseOrbitApi";
  constexpr static uint64_t kOrbitApiUsageCount = 5;
  constexpr static const char* kOrbitApiScopeName = "Scope";
  constexpr static uint32_t kOrbitApiScopeColor = 0x11111111;
  constexpr static uint32_t kOrbitApiScopeGroupId = 1;
  constexpr static const char* kOrbitApiStartName = "Start";
  constexpr static uint32_t kOrbitApiStartColor = 0x22222222;
  constexpr static uint32_t kOrbitApiStartGroupId = 2;
  constexpr static const char* kOrbitApiAsyncStringName = "AsyncString";
  constexpr static uint32_t kOrbitApiAsyncStringColor = 0x33333333;
  constexpr static const char* kOrbitApiStartAsyncName = "StartAsync";
  constexpr static uint32_t kOrbitApiStartAsyncId = 3;
  constexpr static uint32_t kOrbitApiStartAsyncColor = 0x44444444;
  constexpr static const char* kOrbitApiIntName = "Int";
  constexpr static int32_t kOrbitApiIntValue = -42;
  constexpr static uint32_t kOrbitApiIntColor = 0x55555555;
  constexpr static const char* kOrbitApiUintName = "Uint";
  constexpr static int32_t kOrbitApiUintValue = 42;
  constexpr static uint32_t kOrbitApiUintColor = 0x66666666;
  constexpr static const char* kOrbitApiInt64Name = "Int64";
  constexpr static int32_t kOrbitApiInt64Value = -43;
  constexpr static uint32_t kOrbitApiInt64Color = 0x77777777;
  constexpr static const char* kOrbitApiUint64Name = "Uint64";
  constexpr static int32_t kOrbitApiUint64Value = 43;
  constexpr static uint32_t kOrbitApiUint64Color = 0x88888888;
  constexpr static const char* kOrbitApiFloatName = "Float";
  constexpr static int32_t kOrbitApiFloatValue = 44.0f;
  constexpr static uint32_t kOrbitApiFloatColor = 0x99999999;
  constexpr static const char* kOrbitApiDoubleName = "Double";
  constexpr static int32_t kOrbitApiDoubleValue = 45.0;
  constexpr static uint32_t kOrbitApiDoubleColor = 0xAAAAAAAA;

  constexpr static const char* kSleepCommand = "sleep";
  constexpr static const char* kCallOuterFunctionCommand = "CallOuterFunction";
  constexpr static const char* kPthreadSetnameNpCommand = "pthread_setname_np";
  constexpr static const char* kDlopenCommand = "dlopen";
  constexpr static const char* kVulkanTutorialCommand = "VulkanTutorial";
  constexpr static const char* kOrbitApiCommand = "OrbitApi";
  constexpr static const char* kIncreaseRssCommand = "AllocateMemory";

  constexpr static const char* kDoneResponse = "DONE";
};

int IntegrationTestPuppetMain();

}  // namespace orbit_linux_tracing_integration_tests

#endif  // LINUX_TRACING_INTEGRATION_TEST_PUPPET_H_
