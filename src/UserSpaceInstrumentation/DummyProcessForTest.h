// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>

#include <filesystem>
#include <mutex>
#include <set>
#include <thread>

#include "gtest/gtest.h"

namespace orbit_user_space_instrumentation {

// DummyProcessForTest forks a new process in the constructor and starts a multi threaded dummy
// load: A busy loop that spawns and joins threads. The spawned threads perform a busy wait for 15
// ms. Four worker threads are kept active. When DummyProcessForTest goes out of scope the process
// is ended.
class DummyProcessForTest {
 public:
  DummyProcessForTest();
  ~DummyProcessForTest();

  [[nodiscard]] pid_t pid() { return pid_; }

 private:
  // Busy wait for 15 ms.
  void DummyWorker();
  // Busy loop that spawns and joins threads.
  // The spawned threads perform a busy wait for 15 ms. Four worker threads are kept active until
  // flag_file_run_child_ is deleted.
  void DummyWorkload();

  pid_t pid_;
  std::mutex joinable_mutex_;
  std::set<std::thread::id> joinable_threads_;
  std::filesystem::path flag_file_run_child_;
  std::filesystem::path flag_file_child_started_;
};

}  // namespace orbit_user_space_instrumentation
