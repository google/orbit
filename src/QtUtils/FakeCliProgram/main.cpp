// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/strings/string_view.h>

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include "absl/flags/internal/flag.h"

ABSL_FLAG(int, sleep_for_ms, 0, "The program will sleep for X milliseconds");

ABSL_FLAG(bool, infinite_sleep, false, "The program will sleep indefinitely");

ABSL_FLAG(int, exit_code, 0, "The program returns this exit_code");

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);

  std::cout << "Some example output" << std::endl;

  bool infinite_sleep = absl::GetFlag(FLAGS_infinite_sleep);
  if (infinite_sleep) {
    // Wait on a promise which value is never set.
    std::promise<void>().get_future().wait();
  }

  int sleep_time = absl::GetFlag(FLAGS_sleep_for_ms);
  if (sleep_time != 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds{sleep_time});
    std::cout << "Slept for " << sleep_time << "ms" << std::endl;
  }

  return absl::GetFlag(FLAGS_exit_code);
}
