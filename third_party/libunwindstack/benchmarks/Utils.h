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

#pragma once

#include <benchmark/benchmark.h>
#include <stdint.h>

#include <string>

std::string GetBenchmarkFilesDirectory();

std::string GetElfFile();

std::string GetSymbolSortedElfFile();

// GetLargeCompressedFrameElfFile and GetLargeEhFrameElfFile were added to provide larger
// ELF files for more representative benchmarks. Theses ELF files will enable validation
// of optimizations to the unwindstack::Elf.
std::string GetLargeCompressedFrameElfFile();

std::string GetLargeEhFrameElfFile();

#if defined(__BIONIC__)

#include <meminfo/procmeminfo.h>
#include <procinfo/process_map.h>

void GatherRss(uint64_t* rss_bytes);

#endif

class MemoryTracker {
 public:
  void StartTrackingAllocations();
  void StopTrackingAllocations();
  void SetBenchmarkCounters(benchmark::State& state);

 private:
#if defined(__BIONIC__)
  uint64_t total_rss_bytes_ = 0;
  uint64_t min_rss_bytes_ = 0;
  uint64_t max_rss_bytes_ = 0;
  uint64_t rss_bytes_before_;
#endif
  uint64_t total_alloc_bytes_ = 0;
  uint64_t min_alloc_bytes_ = std::numeric_limits<uint64_t>::max();
  uint64_t max_alloc_bytes_ = 0;
  uint64_t alloc_bytes_before_;
  // Benchmarks may run multiple times (the whole benchmark not just what is in the ranged based
  // for loop) but this instance is not destructed and re-constructed each time. So this holds the
  // total number of iterations of the ranged for loop across all runs of a single benchmark.
  size_t total_iterations_ = 0;
};
