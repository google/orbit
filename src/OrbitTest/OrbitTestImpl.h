// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_TEST_ORBIT_TEST_H_
#define ORBIT_TEST_ORBIT_TEST_H_

#include <stdint.h>

#include <memory>
#include <thread>
#include <vector>

#include "OrbitBase/ThreadPool.h"

class OrbitTestImpl {
 public:
  OrbitTestImpl();
  OrbitTestImpl(uint32_t num_threads, uint32_t recurse_depth, uint32_t sleep_us);
  ~OrbitTestImpl();

  void Start();

 private:
  void Init();
  void Loop();
  void TestFunc(uint32_t depth = 0);
  void TestFunc2(uint32_t depth = 0);
  static void BusyWork(uint64_t microseconds);
  void ManualInstrumentationApiTest();
  void OutputOrbitApiState() const;

  bool exit_requested_ = false;
  std::vector<std::shared_ptr<std::thread>> threads_;
  uint32_t num_threads_ = 10;
  uint32_t recurse_depth_ = 10;
  uint32_t sleep_us_ = 100'000;
  std::shared_ptr<orbit_base::ThreadPool> thread_pool_;
};

#endif  // ORBIT_TEST_ORBIT_TEST_IMPL_H_
