// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TestProcess.h"

#include <absl/time/clock.h>

#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include "OrbitBase/Logging.h"

namespace orbit_user_space_instrumentation {

namespace fs = std::filesystem;

namespace {

// Create a file at path.
void Touch(fs::path p) {
  std::ofstream ofs(p);
  ofs << "\n";
}

}  // namespace

TestProcess::TestProcess() {
  flag_file_run_child_ = std::tmpnam(nullptr);
  flag_file_child_started_ = std::tmpnam(nullptr);
  Touch(flag_file_run_child_);
  pid_ = fork();
  CHECK(pid_ != -1);
  // Start the workload and have the parent wait for the startup to complete.
  if (pid_ == 0) {
    Workload();
    exit(0);
  }
  std::error_code error;
  while (!fs::exists(flag_file_child_started_, error)) {
    CHECK(!error);
  };
}

TestProcess::~TestProcess() {
  std::error_code error;
  fs::remove(flag_file_run_child_, error);
  CHECK(!error);
  int status;
  waitpid(pid_, &status, 0);
  CHECK(WIFEXITED(status));
  fs::remove(flag_file_child_started_, error);
  CHECK(!error);
}

void TestProcess::Worker() {
  constexpr auto kTimeToLiveMs = std::chrono::milliseconds(15);
  const auto deadline = std::chrono::system_clock::now() + kTimeToLiveMs;
  while (true) {
    if (std::chrono::system_clock::now() > deadline) {
      break;
    }
  }
  absl::MutexLock lock{&joinable_threads_mutex_};
  joinable_threads_.emplace(std::this_thread::get_id());
}

void TestProcess::Workload() {
  size_t kNumThreads = 4;
  std::vector<std::thread> threads;
  while (fs::exists(flag_file_run_child_) || !threads.empty()) {
    // Spawn as many threads as there are missing.
    while (threads.size() < kNumThreads && fs::exists(flag_file_run_child_)) {
      threads.emplace_back(std::thread(&TestProcess::Worker, this));
    }
    Touch(flag_file_child_started_);
    // Join the finished threads.
    for (auto& t : threads) {
      const auto id = t.get_id();
      absl::MutexLock lock{&joinable_threads_mutex_};
      if (joinable_threads_.count(id) != 0) {
        joinable_threads_.erase(id);
        t.join();
      }
    }
    for (auto it = threads.begin(); it != threads.end();) {
      if (!it->joinable()) {
        it = threads.erase(it);
      } else {
        ++it;
      }
    }
  }
}

}  // namespace orbit_user_space_instrumentation