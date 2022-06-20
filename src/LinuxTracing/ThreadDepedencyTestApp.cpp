// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <iterator>
#include <mutex>
#include <thread>
#include "OrbitBase/ThreadUtils.h"

static std::atomic<bool> exit_requested;
static std::atomic<int> num;

absl::Mutex test_mutex;

constexpr int kNumOfThreads = 4;
constexpr int kSigint = 2;

std::thread my_threads[kNumOfThreads];

void SignalHandler( int signum ) {
  std::cout << "Cleaning up!" << std::endl;
  exit_requested.store(false);
  num++;
  for (auto& my_thread : my_threads) {
    my_thread.join();
  }
  exit(signum);  
}

static void DoWork(int thread_num) {
  std::string thread_name = absl::StrFormat("Thread %d", thread_num);
  orbit_base::SetCurrentThreadName(thread_name.c_str());
  while (true) {
    if (!exit_requested.load()) {
      break;
    }
    int temp = num.load();
    absl::MutexLock test_mutex_lock(&test_mutex);
    while (temp == num.load())
      ;
  }
}

int main() {
  // This program just spawns kNumOfThreads threads and has a shared mutex between all of them.
  // All of the threads are monitoring num. Every 100 millisecond the main thread changes num.
  // When a thread detects that num has changed it unlocks the mutex. One of the kNumOfThreads
  // threads locks the mutex again and waits for another change of num and so on.
  // Output should show kNumOfThreads + 1 different threads when being instrumented by Orbit.
  // One of the threads (main) should only work approx. every 100 ms for a very short while and 
  // then sleep. Only one of the other threads should be running. Every time main runs, the 
  // running non-main thread should change. The new running thread should show the previous 
  // non-main running thread in the tooltip in the "Was Blocked By Thread" field (in the first 
  // blue slice). 
  signal(kSigint, SignalHandler);
  exit_requested.store(true);
  for (int i = 0; i < kNumOfThreads; i++) {
    my_threads[i] = std::thread(&DoWork, i);
  }
  while (num < 10000) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    num++;
  }
  exit_requested.store(false);
  for (auto& my_thread : my_threads) {
    my_thread.join();
  }
}
