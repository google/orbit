// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TestProcess.h"

#include <absl/time/clock.h>

#include <chrono>
#include <string>
#include <system_error>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/WriteStringToFile.h"

namespace orbit_user_space_instrumentation {

namespace fs = std::filesystem;

namespace {

// Create a file at path.
void Touch(const fs::path& path) {
  if (ErrorMessageOr<void> result = orbit_base::WriteStringToFile(path, "\n"); result.has_error()) {
    ERROR("%s", result.error().message());
  }
}

}  // namespace

TestProcess::TestProcess() {
  {
    auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
    CHECK(temporary_file_or_error.has_value());
    flag_file_run_child_.emplace(std::move(temporary_file_or_error.value()));
  }

  {
    auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
    CHECK(temporary_file_or_error.has_value());
    flag_file_child_started_.emplace(std::move(temporary_file_or_error.value()));
  }

  Touch(flag_file_run_child_->file_path());
  flag_file_child_started_->CloseAndRemove();
  pid_ = fork();
  CHECK(pid_ != -1);
  // Start the workload and have the parent wait for the startup to complete.
  if (pid_ == 0) {
    Workload();
    exit(0);
  }
  std::error_code error;
  while (!fs::exists(flag_file_child_started_->file_path(), error)) {
    CHECK(!error);
  };
}

TestProcess::~TestProcess() {
  flag_file_run_child_->CloseAndRemove();
  int status;
  waitpid(pid_, &status, 0);
  CHECK(WIFEXITED(status));
}

void TestProcess::Worker() {
  constexpr auto kTimeToLive = std::chrono::milliseconds(15);
  const auto deadline = std::chrono::system_clock::now() + kTimeToLive;
  while (true) {
    if (std::chrono::system_clock::now() > deadline) {
      break;
    }
  }
  absl::MutexLock lock{&joinable_threads_mutex_};
  joinable_threads_.emplace(std::this_thread::get_id());
}

void TestProcess::Workload() {
  constexpr size_t kNumThreads = 4;
  std::vector<std::thread> threads;
  std::error_code error;
  while (fs::exists(flag_file_run_child_->file_path(), error) || !threads.empty()) {
    CHECK(!error);
    // Spawn as many threads as there are missing.
    while (threads.size() < kNumThreads && fs::exists(flag_file_run_child_->file_path(), error)) {
      CHECK(!error);
      threads.emplace_back(std::thread(&TestProcess::Worker, this));
    }
    Touch(flag_file_child_started_->file_path());
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