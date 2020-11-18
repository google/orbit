// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <vector>

std::mutex g_joinable_mutex;
std::set<std::thread::id> g_joinable_threads;

// Busy wait for ttl_ms.
void Worker(int ttl_ms) {
  auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(ttl_ms);
  while (true) {
    if (std::chrono::system_clock::now() > deadline) {
      break;
    }
  }
  g_joinable_mutex.lock();
  g_joinable_threads.emplace(std::this_thread::get_id());
  g_joinable_mutex.unlock();
}

// Main thread goes into a busy loop that spawns and joins threads.
// The spawned threads go into a busy wait.
// The first command line parameter gives the number of threads to spawn.
// The second command line parameter gives the live time of each spawned thread.
// The actual live time is varied between 100% and 200% of that value to
// make things slightly more interesting.
int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage:\n./OrbitTestShortLivedThreads number_of_threads time_to_live_per_thread\n";
    return -1;
  }

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(1.0, 2.0);

  uint32_t num_threads = std::stoul(argv[1]);
  uint32_t ttl_ms = std::stoul(argv[2]);
  std::vector<std::thread> ts;
  while (true) {
    // Spawn as many threads as there are missing.
    while (num_threads > ts.size()) {
      ts.emplace_back(std::thread(Worker, ttl_ms * dis(gen)));
    }
    // Join the finished threads.
    std::lock_guard lock(g_joinable_mutex);
    for (auto& t : ts) {
      auto id = t.get_id();
      if (g_joinable_threads.count(id) != 0) {
        g_joinable_threads.erase(id);
        t.join();
      }
    }
    for (auto it = ts.begin(); it != ts.end();) {
      if (!it->joinable()) {
        it = ts.erase(it);
      } else {
        ++it;
      }
    }
  }

  return 0;
}
