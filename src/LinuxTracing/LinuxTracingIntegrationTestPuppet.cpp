// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LinuxTracingIntegrationTestPuppet.h"

#include <absl/base/casts.h>
#include <absl/time/clock.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sched.h>
#include <stddef.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"

// This executable is used by LinuxTracingIntegrationTest to test the generation of specific
// perf_event_open event. The behavior is controlled by commands sent on standard input.

namespace orbit_linux_tracing {

using PuppetConstants = LinuxTracingIntegrationTestPuppetConstants;

static void SleepRepeatedly() {
  for (uint64_t i = 0; i < PuppetConstants::kSleepCount; ++i) {
    absl::SleepFor(absl::Microseconds(10));
  }
}

extern "C" __attribute__((noinline)) void InnerFunctionToInstrument() {
  double result = 1;
  for (size_t i = 0; i < 1'000'000; ++i) {
    result = 1 / (2 + result);
  }
  LOG("InnerFunctionToInstrument: %f", 1 + result);
}

extern "C" __attribute__((noinline)) void OuterFunctionToInstrument() {
  for (uint64_t i = 0; i < PuppetConstants::kInnerFunctionCallCount; ++i) {
    InnerFunctionToInstrument();
  }
}

static void CallOuterFunctionToInstrument() {
  for (uint64_t i = 0; i < PuppetConstants::kOuterFunctionCallCount; ++i) {
    OuterFunctionToInstrument();
  }
}

static void ChangeCurrentThreadName() {
  pthread_setname_np(pthread_self(), PuppetConstants::kNewThreadName);
}

static void LoadSoWithDlopenAndCallFunction() {
  constexpr const char* kSoFileName = PuppetConstants::kSharedObjectFileName;
  constexpr const char* kFunctionName = "function_that_works_for_a_considerable_amount_of_time";
  // Setting rpath in CMake is a nightmare, so we are going to emulate "$ORIGIN/../lib" rpath here.
  std::string library_path = (orbit_base::GetExecutableDir() / ".." / "lib" / kSoFileName).string();
  void* handle = dlopen(library_path.c_str(), RTLD_NOW);
  if (handle == nullptr) {
    FATAL("Unable to open \"%s\": %s", kSoFileName, dlerror());
  }

  using function_type = double (*)();
  auto function = absl::bit_cast<function_type>(dlsym(handle, kFunctionName));
  if (function == nullptr) {
    FATAL("Unable to find function \"%s\" in \"%s\": %s", kFunctionName, kSoFileName, dlerror());
  }

  LOG("Function call completed: %f", function());
}

int LinuxTracingIntegrationTestPuppetMain() {
  while (!!std::cin && !std::cin.eof()) {
    std::string command;
    std::getline(std::cin, command);
    if (command.empty()) {
      continue;
    }

    LOG("Puppet received command: %s", command);
    if (command == PuppetConstants::kSleepCommand) {
      SleepRepeatedly();
    } else if (command == PuppetConstants::kCallOuterFunctionCommand) {
      CallOuterFunctionToInstrument();
    } else if (command == PuppetConstants::kPthreadSetnameNpCommand) {
      ChangeCurrentThreadName();
    } else if (command == PuppetConstants::kDlopenCommand) {
      LoadSoWithDlopenAndCallFunction();
    } else {
      ERROR("Unknown command: %s", command);
      continue;
    }

    std::cout << PuppetConstants::kDoneResponse << std::endl;
  }
  return 0;
}

}  // namespace orbit_linux_tracing
