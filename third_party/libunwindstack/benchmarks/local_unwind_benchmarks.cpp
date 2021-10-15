/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <stdint.h>

#include <memory>

#include <benchmark/benchmark.h>

#include <android-base/strings.h>

#include <unwindstack/LocalUnwinder.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsGetLocal.h>
#include <unwindstack/Unwinder.h>
#include "MemoryLocalUnsafe.h"

constexpr size_t kMaxFrames = 32;

struct UnwindData {
  std::shared_ptr<unwindstack::Memory>& process_memory;
  unwindstack::Maps* maps;
  bool resolve_names;
};

size_t LocalCall5(size_t (*func)(void*), void* data) {
  return func(data);
}

size_t LocalCall4(size_t (*func)(void*), void* data) {
  return LocalCall5(func, data);
}

size_t LocalCall3(size_t (*func)(void*), void* data) {
  return LocalCall4(func, data);
}

size_t LocalCall2(size_t (*func)(void*), void* data) {
  return LocalCall3(func, data);
}

size_t LocalCall1(size_t (*func)(void*), void* data) {
  return LocalCall2(func, data);
}

static void Run(benchmark::State& state, size_t (*func)(void*), void* data) {
  for (auto _ : state) {
    if (LocalCall1(func, data) < 5) {
      state.SkipWithError("Failed to unwind.");
    }
  }
}

static size_t Unwind(void* data_ptr) {
  UnwindData* data = reinterpret_cast<UnwindData*>(data_ptr);
  std::unique_ptr<unwindstack::Regs> regs(unwindstack::Regs::CreateFromLocal());
  unwindstack::RegsGetLocal(regs.get());
  unwindstack::Unwinder unwinder(kMaxFrames, data->maps, regs.get(), data->process_memory);
  unwinder.SetResolveNames(data->resolve_names);
  unwinder.Unwind();
  return unwinder.NumFrames();
}

static size_t LocalUnwind(void* unwind_ptr) {
  unwindstack::LocalUnwinder* unwinder = reinterpret_cast<unwindstack::LocalUnwinder*>(unwind_ptr);
  std::vector<unwindstack::LocalFrameData> frame_info;
  unwinder->Unwind(&frame_info, kMaxFrames);
  return frame_info.size();
}

static void BM_local_unwind_uncached_process_memory(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemory(getpid());
  unwindstack::LocalMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = true};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_uncached_process_memory);

static void BM_local_unwind_cached_process_memory(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemoryCached(getpid());
  unwindstack::LocalMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = true};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_cached_process_memory);

static void BM_local_unwind_local_updatable_maps_uncached(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemory(getpid());
  unwindstack::LocalUpdatableMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = true};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_local_updatable_maps_uncached);

static void BM_local_unwind_local_updatable_maps_cached(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemoryCached(getpid());
  unwindstack::LocalUpdatableMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = true};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_local_updatable_maps_cached);

static void BM_local_unwind_local_updatable_maps_thread_cached(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemoryThreadCached(getpid());
  unwindstack::LocalUpdatableMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = true};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_local_updatable_maps_thread_cached);

static void BM_local_unwind_local_unwinder(benchmark::State& state) {
  unwindstack::LocalUnwinder unwinder;
  if (!unwinder.Init()) {
    state.SkipWithError("Failed to init local unwinder.");
  }

  Run(state, LocalUnwind, &unwinder);
}
BENCHMARK(BM_local_unwind_local_unwinder);

static void BM_local_unwind_uncached_process_memory_no_func_names(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemory(getpid());
  unwindstack::LocalMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = false};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_uncached_process_memory_no_func_names);

static void BM_local_unwind_cached_process_memory_no_func_names(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemoryCached(getpid());
  unwindstack::LocalMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = false};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_cached_process_memory_no_func_names);

static void BM_local_unwind_local_updatable_maps_uncached_no_func_names(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemory(getpid());
  unwindstack::LocalUpdatableMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = false};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_local_updatable_maps_uncached_no_func_names);

static void BM_local_unwind_local_updatable_maps_cached_no_func_names(benchmark::State& state) {
  auto process_memory = unwindstack::Memory::CreateProcessMemoryCached(getpid());
  unwindstack::LocalUpdatableMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = false};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_local_updatable_maps_cached_no_func_names);

static void BM_local_unwind_uncached_process_memory_unsafe_reads(benchmark::State& state) {
  std::shared_ptr<unwindstack::Memory> process_memory(new unwindstack::MemoryLocalUnsafe());
  unwindstack::LocalMaps maps;
  if (!maps.Parse()) {
    state.SkipWithError("Failed to parse local maps.");
  }

  UnwindData data = {.process_memory = process_memory, .maps = &maps, .resolve_names = true};
  Run(state, Unwind, &data);
}
BENCHMARK(BM_local_unwind_uncached_process_memory_unsafe_reads);
