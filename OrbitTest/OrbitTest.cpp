// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitTest.h"

#include <stdio.h>

#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <thread>

#include "../Orbit.h"

#if __linux__
#define NO_INLINE __attribute__((noinline))
#else
#define NO_INLINE __declspec(noinline)
#endif

//-----------------------------------------------------------------------------
uint64_t GetThreadID() {
  std::stringstream ss;
  ss << std::this_thread::get_id();
  return std::stoull(ss.str());
}

//-----------------------------------------------------------------------------
void SetThreadName(const std::string& a_Name) {
#if __linux__
  pthread_setname_np(pthread_self(), a_Name.c_str());
#endif
}

//-----------------------------------------------------------------------------
OrbitTest::OrbitTest(uint32_t num_threads, uint32_t recurse_depth,
                     uint32_t sleep_us)
    : num_threads_(num_threads),
      recurse_depth_(recurse_depth),
      sleep_us_(sleep_us) {}

//-----------------------------------------------------------------------------
OrbitTest::~OrbitTest() {
  m_ExitRequested = true;

  for (uint32_t i = 0; i < num_threads_; ++i) {
    m_Threads[i]->join();
  }
}

//-----------------------------------------------------------------------------
void OrbitTest::Start() {
  std::cout << "Starting OrbitTest num_threads: " << num_threads_
            << " recurse_depth: " << recurse_depth_
            << " sleep_us: " << sleep_us_ << std::endl;
  for (uint32_t i = 0; i < num_threads_; ++i) {
    auto thread = std::make_shared<std::thread>(&OrbitTest::Loop, this);
    m_Threads.push_back(thread);
  }

  m_Threads.push_back(std::make_shared<std::thread>(
      &OrbitTest::ManualInstrumentationApiTest, this));
}

//-----------------------------------------------------------------------------
void OrbitTest::Loop() {
  SetThreadName(std::string("OrbitThread_") + std::to_string(GetThreadID()));
  uint32_t count = 0;
  while (!m_ExitRequested) {
    ((++count) & 1) == 0 ? TestFunc() : TestFunc2();
  }
}

//-----------------------------------------------------------------------------
void NO_INLINE OrbitTest::TestFunc(uint32_t a_Depth) {
  if (a_Depth == recurse_depth_) return;
  TestFunc(a_Depth + 1);
  std::this_thread::sleep_for(std::chrono::microseconds(sleep_us_));
}

//-----------------------------------------------------------------------------
void NO_INLINE OrbitTest::TestFunc2(uint32_t a_Depth) {
  if (a_Depth == recurse_depth_) return;
  TestFunc(a_Depth + 1);
  BusyWork(sleep_us_);
}

//-----------------------------------------------------------------------------
void NO_INLINE OrbitTest::BusyWork(uint64_t microseconds) {
  auto start = std::chrono::system_clock::now();
  while (true) {
    auto end = std::chrono::system_clock::now();
    uint64_t us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();
    if (us > microseconds) break;
  }
}

//-----------------------------------------------------------------------------
void OrbitTest::ManualInstrumentationApiTest() {
#if ORBIT_API_ENABLED
  while (!m_ExitRequested) {
    ORBIT_SCOPE("ORBIT_SCOPE_TEST");

    ORBIT_START("ORBIT_START_TEST");
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    ORBIT_STOP();

    ORBIT_START_ASYNC("ORBIT_START_ASYNC_TEST", 0);
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    ORBIT_STOP_ASYNC(0);

    static int int_var = -1000;
    if (++int_var > 1000) int_var = -1000;
    ORBIT_INT("int_var", int_var);

    static int64_t int64_var = -1000;
    if (++int64_var > 1000) int64_var = -1000;
    ORBIT_INT64("int64_var", int64_var);

    static int uint_var = 0;
    if (++uint_var > 1000) uint_var = 0;
    ORBIT_UINT("uint_var", uint_var);

    static uint64_t uint64_var = 0;
    if (++uint64_var > 1000) uint64_var = 0;
    ORBIT_UINT64("uint64_var", uint64_var);

    static float float_var = 0.f;
    static volatile float sinf_coeff = 0.001f;
    ORBIT_FLOAT("float_var", sinf((++float_var) * sinf_coeff));

    static double double_var = 0.f;
    static volatile double sin_coeff = 0.001;
    ORBIT_DOUBLE("float_var", sin((++double_var) * sin_coeff));

    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }
#endif
}
