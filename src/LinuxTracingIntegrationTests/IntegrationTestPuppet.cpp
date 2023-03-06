// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "IntegrationTestPuppet.h"

#include <absl/base/casts.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <dlfcn.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadUtils.h"
#ifdef VULKAN_ENABLED
#include "VulkanTutorial/OffscreenRenderingVulkanTutorial.h"
#endif

// This executable is used by LinuxTracingIntegrationTest to test the generation of specific
// perf_event_open events. The behavior is controlled by commands sent on standard input.

// Hack: Don't use ORBIT_API_INSTANTIATE as it would redefine `struct orbit_api_v2 g_orbit_api`,
// which is already defined by the Introspection target.
void* orbit_api_get_function_table_address_v2() { return &g_orbit_api; }

namespace orbit_linux_tracing_integration_tests {

using PuppetConstants = IntegrationTestPuppetConstants;

static void SleepRepeatedly() {
  for (uint64_t i = 0; i < PuppetConstants::kSleepCount; ++i) {
    absl::SleepFor(absl::Microseconds(100));
  }
}

extern "C" __attribute__((noinline)) double InnerFunctionToInstrument(uint64_t di, uint64_t si,
                                                                      uint64_t dx, uint64_t cx,
                                                                      uint64_t r8, uint64_t r9) {
  ORBIT_LOG("InnerFunctionToInstrument called with args: %u, %u, %u, %u, %u, %u", di, si, dx, cx,
            r8, r9);
  double result = 1;
  for (size_t i = 0; i < 1'000'000; ++i) {
    result = 1 / (2 + result);
  }
  return 1 + result;
}

extern "C" __attribute__((noinline)) uint64_t OuterFunctionToInstrument() {
  for (uint64_t i = 0; i < PuppetConstants::kInnerFunctionCallCount; ++i) {
    ORBIT_LOG("InnerFunctionToInstrument returned: %f",
              InnerFunctionToInstrument(PuppetConstants::kInnerFunctionCallArgs[0],
                                        PuppetConstants::kInnerFunctionCallArgs[1],
                                        PuppetConstants::kInnerFunctionCallArgs[2],
                                        PuppetConstants::kInnerFunctionCallArgs[3],
                                        PuppetConstants::kInnerFunctionCallArgs[4],
                                        PuppetConstants::kInnerFunctionCallArgs[5]));
  }
  return PuppetConstants::kOuterFunctionReturnValue;
}

static void CallOuterFunctionToInstrument() {
  for (uint64_t i = 0; i < PuppetConstants::kOuterFunctionCallCount; ++i) {
    uint64_t result = OuterFunctionToInstrument();
    ORBIT_LOG("OuterFunctionToInstrument returned: %#x", result);
  }
}

static void ChangeCurrentThreadName() {
  orbit_base::SetCurrentThreadName(PuppetConstants::kNewThreadName);
}

static void LoadSoWithDlopenAndCallFunction() {
  constexpr const char* kSoFileName = PuppetConstants::kSharedObjectFileName;
  constexpr const char* kFunctionName = "function_that_works_for_a_considerable_amount_of_time";
  // Setting rpath in CMake is a nightmare, so we are going to emulate "$ORIGIN/../lib" rpath here.
  // But let's try the current directory, too.
  std::vector<std::string> library_paths = {
      (orbit_base::GetExecutableDir() / ".." / "lib" / kSoFileName).string(),
      (orbit_base::GetExecutableDir() / kSoFileName).string()};
  void* handle = nullptr;
  for (const std::string& library_path : library_paths) {
    handle = dlopen(library_path.c_str(), RTLD_NOW);
    if (handle != nullptr) {
      break;
    }
    ORBIT_ERROR("Unable to open \"%s\": %s", library_path, dlerror());
  }
  if (handle == nullptr) {
    ORBIT_FATAL("Unable to find \"%s\"", kSoFileName);
  }

  using function_type = double (*)();
  auto function = absl::bit_cast<function_type>(dlsym(handle, kFunctionName));
  if (function == nullptr) {
    ORBIT_FATAL("Unable to find function \"%s\" in \"%s\": %s", kFunctionName, kSoFileName,
                dlerror());
  }

  ORBIT_LOG("Function call completed: %f", function());
}

#ifdef VULKAN_ENABLED
static void RunVulkanTutorial() {
  orbit_vulkan_tutorial::OffscreenRenderingVulkanTutorial tutorial;
  tutorial.Run(PuppetConstants::kFrameCount);
}
#endif

extern "C" __attribute__((noinline)) void UseOrbitApi() {
  for (uint64_t i = 0; i < PuppetConstants::kOrbitApiUsageCount; ++i) {
    ORBIT_LOG("Using OrbitApi");
    constexpr absl::Duration kDelayBetweenEvents = absl::Microseconds(100);
    {
      ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID(
          PuppetConstants::kOrbitApiScopeName,
          static_cast<orbit_api_color>(PuppetConstants::kOrbitApiScopeColor),
          PuppetConstants::kOrbitApiScopeGroupId);
      absl::SleepFor(kDelayBetweenEvents);
    }
    absl::SleepFor(kDelayBetweenEvents);

    ORBIT_START_WITH_COLOR_AND_GROUP_ID(
        PuppetConstants::kOrbitApiStartName,
        static_cast<orbit_api_color>(PuppetConstants::kOrbitApiStartColor),
        PuppetConstants::kOrbitApiStartGroupId);
    absl::SleepFor(kDelayBetweenEvents);
    ORBIT_STOP();
    absl::SleepFor(kDelayBetweenEvents);

    ORBIT_ASYNC_STRING_WITH_COLOR(
        PuppetConstants::kOrbitApiAsyncStringName, PuppetConstants::kOrbitApiStartAsyncId,
        static_cast<orbit_api_color>(PuppetConstants::kOrbitApiAsyncStringColor));
    absl::SleepFor(kDelayBetweenEvents);
    ORBIT_START_ASYNC_WITH_COLOR(
        PuppetConstants::kOrbitApiStartAsyncName, PuppetConstants::kOrbitApiStartAsyncId,
        static_cast<orbit_api_color>(PuppetConstants::kOrbitApiStartAsyncColor));
    absl::SleepFor(kDelayBetweenEvents);
    ORBIT_STOP_ASYNC(PuppetConstants::kOrbitApiStartAsyncId);
    absl::SleepFor(kDelayBetweenEvents);

    ORBIT_INT_WITH_COLOR(PuppetConstants::kOrbitApiIntName, PuppetConstants::kOrbitApiIntValue,
                         static_cast<orbit_api_color>(PuppetConstants::kOrbitApiIntColor));
    absl::SleepFor(kDelayBetweenEvents);
    ORBIT_UINT_WITH_COLOR(PuppetConstants::kOrbitApiUintName, PuppetConstants::kOrbitApiUintValue,
                          static_cast<orbit_api_color>(PuppetConstants::kOrbitApiUintColor));
    absl::SleepFor(kDelayBetweenEvents);
    ORBIT_INT64_WITH_COLOR(PuppetConstants::kOrbitApiInt64Name,
                           PuppetConstants::kOrbitApiInt64Value,
                           static_cast<orbit_api_color>(PuppetConstants::kOrbitApiInt64Color));
    absl::SleepFor(kDelayBetweenEvents);
    ORBIT_UINT64_WITH_COLOR(PuppetConstants::kOrbitApiUint64Name,
                            PuppetConstants::kOrbitApiUint64Value,
                            static_cast<orbit_api_color>(PuppetConstants::kOrbitApiUint64Color));
    absl::SleepFor(kDelayBetweenEvents);
    ORBIT_FLOAT_WITH_COLOR(PuppetConstants::kOrbitApiFloatName,
                           PuppetConstants::kOrbitApiFloatValue,
                           static_cast<orbit_api_color>(PuppetConstants::kOrbitApiFloatColor));
    absl::SleepFor(kDelayBetweenEvents);
    ORBIT_DOUBLE_WITH_COLOR(PuppetConstants::kOrbitApiDoubleName,
                            PuppetConstants::kOrbitApiDoubleValue,
                            static_cast<orbit_api_color>(PuppetConstants::kOrbitApiDoubleColor));
    absl::SleepFor(kDelayBetweenEvents);
  }
}

void IncreaseRssCommand() {
  void* ptr = malloc(PuppetConstants::kRssIncreaseB);  // NOLINT
  ORBIT_CHECK(ptr != nullptr);
  auto* typed_ptr = static_cast<volatile uint64_t*>(ptr);
  for (uint64_t i = 0; i < PuppetConstants::kRssIncreaseB / sizeof(uint64_t); ++i) {
    typed_ptr[i] = i;
  }
}

int IntegrationTestPuppetMain() {
  ORBIT_LOG("Puppet started");
  while (!!std::cin && !std::cin.eof()) {
    std::string command;
    std::getline(std::cin, command);
    if (command.empty()) {
      continue;
    }

    ORBIT_LOG("Puppet received command: %s", command);
    if (command == PuppetConstants::kSleepCommand) {
      SleepRepeatedly();
    } else if (command == PuppetConstants::kCallOuterFunctionCommand) {
      CallOuterFunctionToInstrument();
    } else if (command == PuppetConstants::kPthreadSetnameNpCommand) {
      ChangeCurrentThreadName();
    } else if (command == PuppetConstants::kDlopenCommand) {
      LoadSoWithDlopenAndCallFunction();
    } else if (command == PuppetConstants::kVulkanTutorialCommand) {
#ifdef VULKAN_ENABLED
      RunVulkanTutorial();
#else
      ORBIT_ERROR("Vulkan isn't enabled. Build with WITH_VULKAN=ON");
#endif
    } else if (command == PuppetConstants::kOrbitApiCommand) {
      UseOrbitApi();
    } else if (command == PuppetConstants::kIncreaseRssCommand) {
      IncreaseRssCommand();
    } else {
      ORBIT_ERROR("Unknown command: %s", command);
      continue;
    }

    std::cout << PuppetConstants::kDoneResponse << std::endl;
  }
  ORBIT_LOG("Puppet finished");
  return 0;
}

}  // namespace orbit_linux_tracing_integration_tests
