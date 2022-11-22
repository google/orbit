// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>

#include "VulkanTutorial/OffscreenRenderingVulkanTutorial.h"

namespace {
std::atomic<bool> exit_requested = false;

void SigintHandler(int signum) {
  if (signum == SIGINT) {
    exit_requested = true;
  }
}

// Use SIGINT to stop the main rendering loop of the VulkanTutorial, transition to its orderly
// shutdown of Vulkan, and exit the program.
void InstallSigintHandler() {
  struct sigaction act {};
  act.sa_handler = SigintHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_restorer = nullptr;
  sigaction(SIGINT, &act, nullptr);
}
}  // namespace

int main() {
  InstallSigintHandler();

  orbit_vulkan_tutorial::OffscreenRenderingVulkanTutorial tutorial;

  std::thread thread{[&tutorial] {
    while (!exit_requested) {
      std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }
    tutorial.StopAsync();
  }};

  tutorial.Run();
  thread.join();

  return 0;
}
