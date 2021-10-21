/*
 * Copyright (C) 2020 The Android Open Source Project
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

#include <err.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdint.h>

#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#include <unwindstack/Elf.h>
#include <unwindstack/Memory.h>

#include "Utils.h"

static void BenchmarkSymbolLookup(benchmark::State& state, std::vector<uint64_t> offsets,
                                  std::string elf_file, bool expect_found, uint32_t runs = 1) {
#if defined(__BIONIC__)
  uint64_t rss_bytes = 0;
#endif
  uint64_t alloc_bytes = 0;
  for (auto _ : state) {
    state.PauseTiming();
#if defined(__BIONIC__)
    mallopt(M_PURGE, 0);
    uint64_t rss_bytes_before = 0;
    GatherRss(&rss_bytes_before);
#endif
    uint64_t alloc_bytes_before = mallinfo().uordblks;
    state.ResumeTiming();

    unwindstack::Elf elf(unwindstack::Memory::CreateFileMemory(elf_file, 0).release());
    if (!elf.Init() || !elf.valid()) {
      errx(1, "Internal Error: Cannot open elf: %s", elf_file.c_str());
    }

    unwindstack::SharedString name;
    uint64_t offset;
    for (size_t i = 0; i < runs; i++) {
      for (auto pc : offsets) {
        bool found = elf.GetFunctionName(pc, &name, &offset);
        if (expect_found && !found) {
          errx(1, "expected pc 0x%" PRIx64 " present, but not found.", pc);
        } else if (!expect_found && found) {
          errx(1, "expected pc 0x%" PRIx64 " not present, but found.", pc);
        }
      }
    }

    state.PauseTiming();
#if defined(__BIONIC__)
    mallopt(M_PURGE, 0);
#endif
    alloc_bytes += mallinfo().uordblks - alloc_bytes_before;
#if defined(__BIONIC__)
    GatherRss(&rss_bytes);
    rss_bytes -= rss_bytes_before;
#endif
    state.ResumeTiming();
  }

#if defined(__BIONIC__)
  state.counters["RSS_BYTES"] = rss_bytes / static_cast<double>(state.iterations());
#endif
  state.counters["ALLOCATED_BYTES"] = alloc_bytes / static_cast<double>(state.iterations());
}

static void BenchmarkSymbolLookup(benchmark::State& state, uint64_t pc, std::string elf_file,
                                  bool expect_found, uint32_t runs = 1) {
  BenchmarkSymbolLookup(state, std::vector<uint64_t>{pc}, elf_file, expect_found, runs);
}

void BM_elf_and_symbol_not_present(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0, GetElfFile(), false);
}
BENCHMARK(BM_elf_and_symbol_not_present);

void BM_elf_and_symbol_find_single(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0x22b2bc, GetElfFile(), true);
}
BENCHMARK(BM_elf_and_symbol_find_single);

void BM_elf_and_symbol_find_single_many_times(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0x22b2bc, GetElfFile(), true, 4096);
}
BENCHMARK(BM_elf_and_symbol_find_single_many_times);

void BM_elf_and_symbol_find_multiple(benchmark::State& state) {
  BenchmarkSymbolLookup(state,
                        std::vector<uint64_t>{0x22b2bc, 0xd5d30, 0x1312e8, 0x13582e, 0x1389c8},
                        GetElfFile(), true);
}
BENCHMARK(BM_elf_and_symbol_find_multiple);

void BM_elf_and_symbol_not_present_from_sorted(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0, GetSymbolSortedElfFile(), false);
}
BENCHMARK(BM_elf_and_symbol_not_present_from_sorted);

void BM_elf_and_symbol_find_single_from_sorted(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0x138638, GetSymbolSortedElfFile(), true);
}
BENCHMARK(BM_elf_and_symbol_find_single_from_sorted);

void BM_elf_and_symbol_find_single_many_times_from_sorted(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0x138638, GetSymbolSortedElfFile(), true, 4096);
}
BENCHMARK(BM_elf_and_symbol_find_single_many_times_from_sorted);

void BM_elf_and_symbol_find_multiple_from_sorted(benchmark::State& state) {
  BenchmarkSymbolLookup(state,
                        std::vector<uint64_t>{0x138638, 0x84350, 0x14df18, 0x1f3a38, 0x1f3ca8},
                        GetSymbolSortedElfFile(), true);
}
BENCHMARK(BM_elf_and_symbol_find_multiple_from_sorted);

void BM_elf_and_symbol_not_present_from_large_compressed_frame(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0, GetLargeCompressedFrameElfFile(), false);
}
BENCHMARK(BM_elf_and_symbol_not_present_from_large_compressed_frame);

void BM_elf_and_symbol_find_single_from_large_compressed_frame(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0x202aec, GetLargeCompressedFrameElfFile(), true);
}
BENCHMARK(BM_elf_and_symbol_find_single_from_large_compressed_frame);

void BM_elf_and_symbol_find_single_many_times_from_large_compressed_frame(benchmark::State& state) {
  BenchmarkSymbolLookup(state, 0x202aec, GetLargeCompressedFrameElfFile(), true, 4096);
}
BENCHMARK(BM_elf_and_symbol_find_single_many_times_from_large_compressed_frame);

void BM_elf_and_symbol_find_multiple_from_large_compressed_frame(benchmark::State& state) {
  BenchmarkSymbolLookup(state,
                        std::vector<uint64_t>{0x202aec, 0x23e74c, 0xd000c, 0x201b10, 0x183060},
                        GetLargeCompressedFrameElfFile(), true);
}
BENCHMARK(BM_elf_and_symbol_find_multiple_from_large_compressed_frame);
