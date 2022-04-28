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

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <unistd.h>

#include <memory>

#include <benchmark/benchmark.h>

#include <unwindstack/AndroidUnwinder.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/Unwinder.h>

#include "MemoryRemote.h"
#include "PidUtils.h"
#include "tests/TestUtils.h"

static bool WaitForRemote(pid_t pid, volatile bool* ready_ptr) {
  return unwindstack::RunWhenQuiesced(pid, true, [pid, ready_ptr]() {
    unwindstack::MemoryRemote memory(pid);
    bool ready;
    uint64_t ready_addr = reinterpret_cast<uint64_t>(ready_ptr);
    if (memory.ReadFully(ready_addr, &ready, sizeof(ready)) && ready) {
      return unwindstack::PID_RUN_PASS;
    }
    return unwindstack::PID_RUN_KEEP_GOING;
  });
}

size_t RemoteCall6(volatile bool* ready) {
  *ready = true;
  while (true)
    ;
}

size_t RemoteCall5(volatile bool* ready) {
  return RemoteCall6(ready);
}

size_t RemoteCall4(volatile bool* ready) {
  return RemoteCall5(ready);
}

size_t RemoteCall3(volatile bool* ready) {
  return RemoteCall4(ready);
}

size_t RemoteCall2(volatile bool* ready) {
  return RemoteCall3(ready);
}

size_t RemoteCall1(volatile bool* ready) {
  return RemoteCall2(ready);
}

static pid_t StartRemoteRun() {
  static volatile bool ready = false;

  pid_t pid;
  if ((pid = fork()) == 0) {
    RemoteCall1(&ready);
    exit(0);
  }
  if (pid == -1) {
    return -1;
  }

  if (!WaitForRemote(pid, &ready)) {
    kill(pid, SIGKILL);
    waitpid(pid, nullptr, 0);
    return -1;
  }

  return pid;
}

static void RemoteUnwind(benchmark::State& state, bool cached) {
  pid_t pid = StartRemoteRun();
  if (pid == -1) {
    state.SkipWithError("Failed to start remote process.");
  }
  unwindstack::TestScopedPidReaper reap(pid);

  std::shared_ptr<unwindstack::Memory> process_memory;
  if (cached) {
    process_memory = unwindstack::Memory::CreateProcessMemoryCached(pid);
  } else {
    process_memory = unwindstack::Memory::CreateProcessMemory(pid);
  }
  unwindstack::RemoteMaps maps(pid);
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse maps.");
  }

  for (auto _ : state) {
    std::unique_ptr<unwindstack::Regs> regs(unwindstack::Regs::RemoteGet(pid));
    unwindstack::Unwinder unwinder(32, &maps, regs.get(), process_memory);
    unwinder.Unwind();
    if (unwinder.NumFrames() < 5) {
      state.SkipWithError("Failed to unwind properly.");
    }
  }

  ptrace(PTRACE_DETACH, pid, 0, 0);
}

static void BM_remote_unwind_uncached(benchmark::State& state) {
  RemoteUnwind(state, false);
}
BENCHMARK(BM_remote_unwind_uncached);

static void BM_remote_unwind_cached(benchmark::State& state) {
  RemoteUnwind(state, true);
}
BENCHMARK(BM_remote_unwind_cached);

static void RemoteAndroidUnwind(benchmark::State& state, bool cached) {
  pid_t pid = StartRemoteRun();
  if (pid == -1) {
    state.SkipWithError("Failed to start remote process.");
  }
  unwindstack::TestScopedPidReaper reap(pid);

  std::shared_ptr<unwindstack::Memory> process_memory;
  if (cached) {
    process_memory = unwindstack::Memory::CreateProcessMemoryCached(pid);
  } else {
    process_memory = unwindstack::Memory::CreateProcessMemory(pid);
  }
  unwindstack::AndroidRemoteUnwinder unwinder(pid, process_memory);
  unwindstack::ErrorData error;
  if (!unwinder.Initialize(error)) {
    state.SkipWithError("Failed to initialize unwinder.");
  }

  for (auto _ : state) {
    unwindstack::AndroidUnwinderData data;
    if (!unwinder.Unwind(data) || data.frames.size() < 5) {
      state.SkipWithError("Failed to unwind properly.");
    }
  }

  ptrace(PTRACE_DETACH, pid, 0, 0);
}

static void BM_remote_android_unwind_uncached(benchmark::State& state) {
  RemoteAndroidUnwind(state, true);
}
BENCHMARK(BM_remote_android_unwind_uncached);

static void BM_remote_android_unwind_cached(benchmark::State& state) {
  RemoteAndroidUnwind(state, true);
}
BENCHMARK(BM_remote_android_unwind_cached);
