// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/synchronization/mutex.h>
#include <unistd.h>

#include <filesystem>
#include <optional>
#include <set>
#include <thread>

#include "TestUtils/TemporaryFile.h"

namespace orbit_user_space_instrumentation {

// TestProcess forks a new process in the constructor and starts a multi threaded load: A busy loop
// that spawns and joins threads. The spawned threads perform a busy wait for 15 ms. Four worker
// threads are kept active. When TestProcess goes out of scope the process is ended.
class TestProcess {
 public:
  TestProcess();
  virtual ~TestProcess();

  [[nodiscard]] pid_t pid() { return pid_; }

 private:
  // Busy wait for 15 ms.
  void Worker();
  // Busy loop that spawns and joins threads.
  // The spawned threads perform a busy wait for 15 ms. Four worker threads are kept active until
  // flag_file_run_child_ is deleted.
  void Workload();

  pid_t pid_ = 0;
  absl::Mutex joinable_threads_mutex_;
  std::set<std::thread::id> joinable_threads_;
  std::optional<orbit_test_utils::TemporaryFile> flag_file_run_child_;
  std::optional<orbit_test_utils::TemporaryFile> flag_file_child_started_;
};
}  // namespace orbit_user_space_instrumentation
