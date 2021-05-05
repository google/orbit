// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentationTestLib.h"

#include <stdio.h>

#include <chrono>

int TrivialFunction() { return 42; }

uint64_t TrivialSum(uint64_t p0, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5) {
  return p0 + p1 + p2 + p3 + p4 + p5;
}

void TrivialLog(uint64_t function_address) {
  using std::chrono::system_clock;
  constexpr std::chrono::duration<int, std::ratio<1>> kOneSecond(1);
  constexpr std::chrono::duration<int, std::ratio<1, 1000>> kThreeMilliseconds(3);
  static system_clock::time_point last_logged_event = system_clock::now() - kOneSecond;
  static uint64_t skipped = 0;
  // Rate limit log output to once every three milliseconds.
  const system_clock::time_point now = system_clock::now();
  if (now - last_logged_event > kThreeMilliseconds) {
    if (skipped > 0) {
      printf(" ( %lu skipped events )\n", skipped);
    }
    printf("Called function at %#lx\n", function_address);
    last_logged_event = now;
    skipped = 0;
  } else {
    skipped++;
  }
}
