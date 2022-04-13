// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// g++ target.cc -O0 -fno-omit-frame-pointer -momit-leaf-frame-pointer -o target_fp
// g++ target.cc -O0 -fomit-frame-pointer -o target_no_fp

#include <sched.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>

inline uint64_t timestamp_ns() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return 1'000'000'000llu * ts.tv_sec + ts.tv_nsec;
}

uint64_t every_1us() {
  uint64_t result = 0;
  for (int i = 0; i < 342; ++i) {  // 333 on gamelet
    result += i;
  }
  return result;
}

uint64_t every_10us() {
  uint64_t result = 0;
  for (int i = 0; i < 10; ++i) {
    result += every_1us();
  }
  return result;
}

uint64_t every_100us() {
  uint64_t result = 0;
  for (int i = 0; i < 10; ++i) {
    result += every_10us();
  }
  return result;
}

uint64_t every_1000us() {
  uint64_t result = 0;
  for (int i = 0; i < 10; ++i) {
    result += every_100us();
  }
  return result;
}

uint64_t every_10000us() {
  uint64_t result = 0;
  for (int i = 0; i < 10; ++i) {
    result += every_1000us();
  }
  return result;
}

uint64_t every_100000us() {
  uint64_t result = 0;
  for (int i = 0; i < 10; ++i) {
    result += every_10000us();
  }
  return result;
}

uint64_t every_1000000us() {
  uint64_t result = 0;
  for (int i = 0; i < 10; ++i) {
    result += every_100000us();
  }
  return result;
}

uint64_t stack_filler(int i) {
  if (i > 0) {
    return stack_filler(i - 1);
  } else {
    return every_1000000us();
  }
}

uint64_t fill_stack(uint64_t bytes) {
  int i = static_cast<int>(bytes / 0x20);
  return stack_filler(i);
}

int main(int argc, char* argv[]) {
  if (true) {
    cpu_set_t cpu_set{};
    CPU_SET(0, &cpu_set);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_set) != 0) {
      fprintf(stderr, "sched_setaffinity error: %s\n", strerror(errno));
    }
  }

  std::vector<double> totals{};
  while (true) {
    uint64_t start = timestamp_ns();
    uint64_t result = fill_stack(0);
    uint64_t end = timestamp_ns();
    double total_us = (end - start) / 1000.0;

    totals.push_back(total_us);
    constexpr int AVG_WINDOW = 10;
    double avg = 0;
    int avg_window = totals.size() < AVG_WINDOW ? totals.size() : AVG_WINDOW;
    for (int i = totals.size() - avg_window; i < totals.size(); ++i) {
      avg += totals[i];
    }
    avg /= avg_window;

    printf("%11.3f, %11.3f\n", total_us, avg_window, avg);
  }
  return 0;
}
