// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "LegacyOrbit.h"
#include "absl/strings/str_format.h"

// AsyncScopeTester starts async scopes in one thread and stops them in another.
class AsyncScopeTester {
 public:
  void Start(const char* name) {
    static uint64_t id = 0;
    ORBIT_START_ASYNC(name, ++id);
    std::lock_guard lock(mutex_);
    async_scope_ids_to_stop_.push_back(id);
  }

  void Stop() {
    std::lock_guard lock(mutex_);
    for (uint64_t id : async_scope_ids_to_stop_) {
      std::string str = absl::StrFormat(
          "This is a very long dynamic string: The quick brown fox jumps over the lazy dog. This "
          "string is associated with task id %u.",
          id);
      ORBIT_ASYNC_STRING(str.c_str(), id);
      ORBIT_STOP_ASYNC(id);

      // Scatter end times for async events.
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    async_scope_ids_to_stop_.clear();
  }

 private:
  std::vector<uint64_t> async_scope_ids_to_stop_;
  std::mutex mutex_;
};

void StartAsyncScopesThread(AsyncScopeTester* tester) {
  while (true) {
    tester->Start("ASYNC_SCOPES_0");
    tester->Start("ASYNC_SCOPES_1");
    tester->Start("ASYNC_SCOPES_2");

    // Throttle the number of generated async events.
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}

void StopAsyncScopesThread(AsyncScopeTester* tester) {
  while (true) {
    tester->Stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(64));
  }
}

static void SleepFor1Ms() {
  ORBIT_SCOPE("SleepFor1Ms");
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

static void SleepFor2Ms() {
  ORBIT_SCOPE("Sleep for two milliseconds");
  ORBIT_SCOPE_WITH_COLOR("Sleep for two milliseconds", orbit::Color::kTeal);
  ORBIT_SCOPE_WITH_COLOR("Sleep for two milliseconds", orbit::Color::kOrange);
  SleepFor1Ms();
  SleepFor1Ms();
}

void ManualInstrumentationApiTest() {
  while (true) {
    ORBIT_SCOPE("legacy_scope_test");
    ORBIT_SCOPE_WITH_COLOR("legacy_scope_test_with_color", orbit::Color(0xff0000ff));

    SleepFor2Ms();

    ORBIT_START_WITH_COLOR("legacy_start_test", orbit::Color::kRed);
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    ORBIT_STOP();

    ORBIT_START_ASYNC_WITH_COLOR("ORBIT_LEGACY_START_ASYNC_TEST", 0, orbit::Color::kLightBlue);
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    ORBIT_STOP_ASYNC(0);

    static int int_var = -100;
    if (++int_var > 100) int_var = -100;
    ORBIT_INT("legacy_int_var", int_var);

    static int64_t int64_var = -100;
    if (++int64_var > 100) int64_var = -100;
    ORBIT_INT64("legacy_int64_var", int64_var);

    static int uint_var = 0;
    if (++uint_var > 100) uint_var = 0;
    ORBIT_UINT("legacy_uint_var", uint_var);

    static uint64_t uint64_var = 0;
    if (++uint64_var > 100) uint64_var = 0;
    ORBIT_UINT64_WITH_COLOR("legacy_uint64_var", uint64_var, orbit::Color::kIndigo);

    static float float_var = 0.f;
    static volatile float sinf_coeff = 0.1f;
    ORBIT_FLOAT_WITH_COLOR("legacy_float_var", sinf((++float_var) * sinf_coeff),
                           orbit::Color::kPink);

    static double double_var = 0.0;
    static volatile double cos_coeff = 0.1;
    ORBIT_DOUBLE_WITH_COLOR("legacy_double_var", cos((++double_var) * cos_coeff),
                            orbit::Color::kPurple);
  }
}

// Program to test support for legacy manual instrumentation API (stand-alone version of Orbit.h).
int main() {
  AsyncScopeTester tester;

  std::thread async_start_thread([&tester] { StartAsyncScopesThread(&tester); });
  std::thread async_stop_thread([&tester] { StopAsyncScopesThread(&tester); });
  std::thread instrumentation_thread([] { ManualInstrumentationApiTest(); });

  async_start_thread.join();
  async_stop_thread.join();
  instrumentation_thread.join();
}
