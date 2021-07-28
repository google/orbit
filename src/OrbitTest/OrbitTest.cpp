// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>
#include <stdio.h>

#include <memory>
#include <string>

#include "OrbitTestImpl.h"

int main(int argc, char* argv[]) {
  std::unique_ptr<OrbitTestImpl> test;
  if (argc == 4) {
    uint32_t num_threads = std::stoul(argv[1]);
    uint32_t recurse_depth = std::stoul(argv[2]);
    uint32_t sleep_us = std::stoul(argv[3]);
    test = std::make_unique<OrbitTestImpl>(num_threads, recurse_depth, sleep_us);
  } else {
    test = std::make_unique<OrbitTestImpl>();
  }

  test->Start();
  getchar();
}
