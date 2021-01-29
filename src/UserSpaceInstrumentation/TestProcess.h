// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/synchronization/mutex.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <filesystem>
#include <set>
#include <thread>

namespace orbit_user_space_instrumentation {

// TestProcess forks a new process in the constructor and starts a multi threaded dummy load: A busy
// loop that spawns and joins threads. The spawned threads perform a busy wait for 15 ms. Four
// worker threads are kept active. When TestProcess goes out of scope the process is ended.
class TestProcess {
 public:
  TestProcess();
  ~TestProcess();

  [[nodiscard]] pid_t pid() { return pid_; }

 private:
  // Busy wait for 15 ms.
  void DummyWorker();
  // Busy loop that spawns and joins threads.
  // The spawned threads perform a busy wait for 15 ms. Four worker threads are kept active until
  // flag_file_run_child_ is deleted.
  void DummyWorkload();

  pid_t pid_;
  absl::Mutex joinable_threads_mutex_;
  std::set<std::thread::id> joinable_threads_;
  std::filesystem::path flag_file_run_child_;
  std::filesystem::path flag_file_child_started_;
};

}  // namespace orbit_user_space_instrumentation
