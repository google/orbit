// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitTestImpl.h"

#include <absl/strings/str_format.h>
#include <absl/time/time.h>
#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>

#include "ApiInterface/Orbit.h"
#include "OrbitBase/ThreadUtils.h"
ORBIT_API_INSTANTIATE;

#if __linux__
#define NO_INLINE __attribute__((noinline))
#else
#define NO_INLINE __declspec(noinline)
#endif

#define ORBIT_SCOPE_FUNCTION ORBIT_SCOPE(__FUNCTION__)

OrbitTestImpl::OrbitTestImpl() { Init(); }

OrbitTestImpl::OrbitTestImpl(uint32_t num_threads, uint32_t recurse_depth, uint32_t sleep_us)
    : num_threads_(num_threads), recurse_depth_(recurse_depth), sleep_us_(sleep_us) {
  Init();
}

void OrbitTestImpl::Init() {
  const size_t kMinNumWorkers = 10;
  const size_t kMaxNumWorkers = 100;
  thread_pool_ =
      orbit_base::ThreadPool::Create(kMinNumWorkers, kMaxNumWorkers, absl::Milliseconds(500));
}

OrbitTestImpl::~OrbitTestImpl() {
  exit_requested_ = true;

  for (uint32_t i = 0; i < num_threads_; ++i) {
    threads_[i]->join();
  }
}

void OrbitTestImpl::Start() {
  std::cout << "Starting OrbitTest num_threads: " << num_threads_
            << " recurse_depth: " << recurse_depth_ << " sleep_us: " << sleep_us_ << std::endl;
  for (uint32_t i = 0; i < num_threads_; ++i) {
    auto thread = std::make_shared<std::thread>(&OrbitTestImpl::Loop, this);
    threads_.push_back(thread);
  }

  threads_.push_back(
      std::make_shared<std::thread>(&OrbitTestImpl::ManualInstrumentationApiTest, this));
}

void OrbitTestImpl::Loop() {
  auto tid = orbit_base::GetCurrentThreadId();
  orbit_base::SetCurrentThreadName(absl::StrFormat("OrbitThread_%s", std::to_string(tid)).c_str());
  uint32_t count = 0;
  while (!exit_requested_) {
    ((++count) & 1) == 0 ? TestFunc() : TestFunc2();
  }
}

void NO_INLINE OrbitTestImpl::TestFunc(uint32_t depth) {
  ORBIT_SCOPE_FUNCTION;
  if (depth == recurse_depth_) return;
  TestFunc(depth + 1);
  std::this_thread::sleep_for(std::chrono::microseconds(sleep_us_));
}

void NO_INLINE OrbitTestImpl::TestFunc2(uint32_t depth) {
  ORBIT_SCOPE_FUNCTION;
  if (depth == recurse_depth_) return;
  TestFunc(depth + 1);
  BusyWork(sleep_us_);
}

void NO_INLINE OrbitTestImpl::BusyWork(uint64_t microseconds) {
  ORBIT_SCOPE_FUNCTION;
  auto start = std::chrono::system_clock::now();
  while (true) {
    auto end = std::chrono::system_clock::now();
    uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    if (us > microseconds) break;
  }
}

#if ORBIT_API_ENABLED
static void NO_INLINE SleepFor1Ms() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }

static void NO_INLINE SleepFor2Ms() {
  ORBIT_SCOPE("Sleep for two milliseconds");
  ORBIT_SCOPE_WITH_COLOR("Sleep for two milliseconds", kOrbitColorTeal);
  ORBIT_SCOPE_WITH_COLOR("Sleep for two milliseconds", kOrbitColorOrange);
  thread_local uint64_t group_id = 0;
  uint64_t current_id = group_id++;
  ORBIT_SCOPE_WITH_GROUP_ID("Sleeping for two milliseconds with group id", current_id);
  ORBIT_SCOPE_WITH_COLOR_AND_GROUP_ID("Sleeping for two milliseconds with group id",
                                      kOrbitColorBlueGrey, current_id);
  SleepFor1Ms();
  SleepFor1Ms();
}

static void ExecuteTask(uint32_t id) {
  static const std::vector<uint32_t> sleep_times_ms = {10, 200, 20,  300, 60,  100, 150,
                                                       20, 30,  320, 380, 400, 450, 500};
  uint32_t sleep_time = sleep_times_ms[id % sleep_times_ms.size()];
  std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
  std::string str = absl::StrFormat(
      "This is a very long dynamic string: The quick brown fox jumps over the lazy dog. This "
      "string is associated with task id %u. We slept for %u ms.",
      id, sleep_time);
  ORBIT_ASYNC_STRING(str.c_str(), id);
  ORBIT_STOP_ASYNC(id);
}

void OrbitTestImpl::OutputOrbitApiState() {
  while (!exit_requested_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    LOG("g_orbit_api_v1.enabled = %u", g_orbit_api_v1.enabled);
  }
}

void OrbitTestImpl::ManualInstrumentationApiTest() {
  thread_pool_->Schedule([this]() { OutputOrbitApiState(); });
  while (!exit_requested_) {
    ORBIT_SCOPE("ORBIT_SCOPE_TEST");
    ORBIT_SCOPE_WITH_COLOR("ORBIT_SCOPE_TEST_WITH_COLOR", orbit_api_color(0xff0000ff));
    SleepFor2Ms();

    ORBIT_START_WITH_COLOR("ORBIT_START_TEST", kOrbitColorRed);
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    ORBIT_STOP();

    thread_local uint64_t group_id = 0;
    uint64_t current_id = group_id++;
    ORBIT_START_WITH_GROUP_ID("ORBIT_START_TEST with group id", current_id);
    ORBIT_START_WITH_COLOR_AND_GROUP_ID("ORBIT_START_TEST with group id", kOrbitColorBlueGrey,
                                        current_id);
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    ORBIT_STOP();
    ORBIT_STOP();

    ORBIT_START_ASYNC_WITH_COLOR("ORBIT_START_ASYNC_TEST", 0, kOrbitColorLightBlue);
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    ORBIT_STOP_ASYNC(0);

    static int int_var = -100;
    if (++int_var > 100) int_var = -100;
    ORBIT_INT("int_var", int_var);

    static int64_t int64_var = -100;
    if (++int64_var > 100) int64_var = -100;
    ORBIT_INT64("int64_var", int64_var);

    static int uint_var = 0;
    if (++uint_var > 100) uint_var = 0;
    ORBIT_UINT("uint_var", uint_var);

    static uint64_t uint64_var = 0;
    if (++uint64_var > 100) uint64_var = 0;
    ORBIT_UINT64_WITH_COLOR("uint64_var", uint64_var, kOrbitColorIndigo);

    static float float_var = 0.f;
    static volatile float sinf_coeff = 0.1f;
    ORBIT_FLOAT_WITH_COLOR("float_var", sinf((++float_var) * sinf_coeff), kOrbitColorPink);

    static double double_var = 0.0;
    static volatile double cos_coeff = 0.1;
    ORBIT_DOUBLE_WITH_COLOR("double_var", cos((++double_var) * cos_coeff), kOrbitColorPurple);

    for (int i = 0; i < 5; ++i) {
      std::string track_name = absl::StrFormat("DynamicName_%u", i);
      ORBIT_DOUBLE(track_name.c_str(), cos(double_var * static_cast<double>(i)));
    }

    // Async spans.
    static uint32_t task_id = 0;
    size_t kNumTasksToSchedule = 10;
    for (size_t i = 0; i < kNumTasksToSchedule; ++i) {
      uint32_t id = ++task_id;
      ORBIT_START_ASYNC("ORBIT_ASYNC_TASKS", id);
      thread_pool_->Schedule([id]() { ExecuteTask(id); });
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
}
#else
void OrbitTest::ManualInstrumentationApiTest() {}
#endif  // ORBIT_API_ENABLED
