/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <signal.h>
#include <stdint.h>

#include <atomic>
#include <thread>

#include <android-base/threads.h>
#include <benchmark/benchmark.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Unwinder.h>

constexpr size_t kMaxFrames = 32;

void ThreadCall6(std::atomic_int* tid, std::atomic_bool* done) {
  tid->store(android::base::GetThreadId());

  while (!done->load()) {
  }
}

void ThreadCall5(std::atomic_int* tid, std::atomic_bool* done) {
  ThreadCall6(tid, done);
}

void ThreadCall4(std::atomic_int* tid, std::atomic_bool* done) {
  ThreadCall5(tid, done);
}

void ThreadCall3(std::atomic_int* tid, std::atomic_bool* done) {
  ThreadCall4(tid, done);
}

void ThreadCall2(std::atomic_int* tid, std::atomic_bool* done) {
  ThreadCall3(tid, done);
}

void ThreadCall1(std::atomic_int* tid, std::atomic_bool* done) {
  ThreadCall2(tid, done);
}

static void BM_thread_unwind(benchmark::State& state) {
  std::atomic_int tid(0);
  std::atomic_bool done;

  // Create the thread before the unwinder object so all maps are no
  // longer changing.
  std::thread thread([&tid, &done] { ThreadCall1(&tid, &done); });

  while (tid.load() == 0) {
  }

  unwindstack::ThreadUnwinder unwinder(kMaxFrames);
  if (!unwinder.Init()) {
    state.SkipWithError("Failed to init.");
  }

  for (auto _ : state) {
    unwinder.UnwindWithSignal(SIGRTMIN, tid.load());
    if (unwinder.NumFrames() < 5) {
      state.SkipWithError("Failed to unwind.");
    }
  }

  done.store(true);
  thread.join();
}
BENCHMARK(BM_thread_unwind);
