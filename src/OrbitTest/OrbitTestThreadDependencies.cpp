// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <stdio.h>

#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#include "OrbitBase/ThreadUtils.h"

static std::atomic<bool> exit_requested;
static std::atomic<int> frame_number;

std::mutex mutex;

constexpr int kNumOfThreads = 4;

void DoWork(int thread_number) {
  std::string thread_name = absl::StrFormat("Worker thread %d", thread_number);
  orbit_base::SetCurrentThreadName(thread_name.c_str());
  while (true) {
    if (exit_requested.load()) {
      break;
    }
    int current_frame_number = frame_number;
    std::lock_guard<std::mutex> lock(mutex);
    int i = 0;
    while (current_frame_number == frame_number) {
      ++i;
      if (i % 1000 == 0) {
        printf("Worker thread %d: At iteration %d", thread_number, i);
      }
    }
  }
}

int main() {
  std::array<std::thread, kNumOfThreads> worker_threads;
  exit_requested.store(false);
  for (int i = 0; i < kNumOfThreads; i++) {
    worker_threads.at(i) = std::thread(&DoWork, i);
  }
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
    frame_number++;
  }
  exit_requested.store(true);
  for (auto& worker_thread : worker_threads) {
    worker_thread.join();
  }
}