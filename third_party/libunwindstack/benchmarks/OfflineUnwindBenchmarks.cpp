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

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <benchmark/benchmark.h>

#include <unwindstack/Arch.h>
#include <unwindstack/Unwinder.h>

#include "Utils.h"
#include "utils/OfflineUnwindUtils.h"

// This collection of benchmarks exercises Unwinder::Unwind for offline unwinds.
//
// See `libunwindstack/utils/OfflineUnwindUtils.h` for more info on offline unwinds
// and b/192012600 for additional information regarding these benchmarks.
namespace unwindstack {
namespace {

static constexpr char kStartup[] = "startup_case";
static constexpr char kSteadyState[] = "steady_state_case";

class OfflineUnwindBenchmark : public benchmark::Fixture {
 public:
  void SetUp(benchmark::State& state) override {
    // Ensure each benchmarks has a fresh ELF cache at the start.
    unwind_case_ = state.range(0) ? kSteadyState : kStartup;
    resolve_names_ = state.range(1);
    Elf::SetCachingEnabled(false);
  }

  void TearDown(benchmark::State& state) override {
    offline_utils_.ReturnToCurrentWorkingDirectory();
    mem_tracker_.SetBenchmarkCounters(state);
  }

  void SingleUnwindBenchmark(benchmark::State& state, const UnwindSampleInfo& sample_info) {
    std::string error_msg;
    if (!offline_utils_.Init(sample_info, &error_msg)) {
      state.SkipWithError(error_msg.c_str());
      return;
    }
    BenchmarkOfflineUnwindMultipleSamples(state, std::vector<UnwindSampleInfo>{sample_info});
  }

  void ConsecutiveUnwindBenchmark(benchmark::State& state,
                                  const std::vector<UnwindSampleInfo>& sample_infos) {
    std::string error_msg;
    if (!offline_utils_.Init(sample_infos, &error_msg)) {
      state.SkipWithError(error_msg.c_str());
      return;
    }
    BenchmarkOfflineUnwindMultipleSamples(state, sample_infos);
  }

 private:
  void BenchmarkOfflineUnwindMultipleSamples(benchmark::State& state,
                                             const std::vector<UnwindSampleInfo>& sample_infos) {
    std::string error_msg;
    auto offline_unwind_multiple_samples = [&](bool benchmarking_unwind) {
      // The benchmark should only measure the time / memory usage for the creation of
      // each Unwinder object and the corresponding unwind as close as possible.
      if (benchmarking_unwind) state.PauseTiming();

      std::unordered_map<std::string_view, std::unique_ptr<Regs>> regs_copies;
      for (const auto& sample_info : sample_infos) {
        const std::string& sample_name = sample_info.offline_files_dir;

        // Need to init unwinder with new copy of regs each iteration because unwinding changes
        // the attributes of the regs object.
        regs_copies.emplace(sample_name,
                            std::unique_ptr<Regs>(offline_utils_.GetRegs(sample_name)->Clone()));

        // The Maps object will still hold the parsed maps from the previous unwinds. So reset them
        // unless we want to assume all Maps are cached.
        if (!sample_info.create_maps) {
          if (!offline_utils_.CreateMaps(&error_msg, sample_name)) {
            state.SkipWithError(error_msg.c_str());
            return;
          }

          // Since this maps object will be cached, need to make sure that
          // all of the names are fully qualified paths. This allows the
          // caching mechanism to properly cache elf files that are
          // actually the same.
          if (!offline_utils_.ChangeToSampleDirectory(&error_msg, sample_name)) {
            state.SkipWithError(error_msg.c_str());
            return;
          }
          for (auto& map_info : *offline_utils_.GetMaps(sample_name)) {
            auto& name = map_info->name();
            if (!name.empty()) {
              std::filesystem::path path;
              if (std::filesystem::is_symlink(name.c_str())) {
                path = std::filesystem::read_symlink(name.c_str());
              } else {
                path = std::filesystem::current_path();
                path /= name.c_str();
              }
              name = path.lexically_normal().c_str();
            }
          }
        }
      }

      if (benchmarking_unwind) mem_tracker_.StartTrackingAllocations();
      for (const auto& sample_info : sample_infos) {
        const std::string& sample_name = sample_info.offline_files_dir;
        // Need to change to sample directory for Unwinder to properly init ELF objects.
        // See more info at OfflineUnwindUtils::ChangeToSampleDirectory.
        if (!offline_utils_.ChangeToSampleDirectory(&error_msg, sample_name)) {
          state.SkipWithError(error_msg.c_str());
          return;
        }
        if (benchmarking_unwind) state.ResumeTiming();

        Unwinder unwinder(128, offline_utils_.GetMaps(sample_name),
                          regs_copies.at(sample_name).get(),
                          offline_utils_.GetProcessMemory(sample_name));
        if (sample_info.memory_flag == ProcessMemoryFlag::kIncludeJitMemory) {
          unwinder.SetJitDebug(offline_utils_.GetJitDebug(sample_name));
        }
        unwinder.SetResolveNames(resolve_names_);
        unwinder.Unwind();

        if (benchmarking_unwind) state.PauseTiming();
        size_t expected_num_frames;
        if (!offline_utils_.GetExpectedNumFrames(&expected_num_frames, &error_msg, sample_name)) {
          state.SkipWithError(error_msg.c_str());
          return;
        }
        if (unwinder.NumFrames() != expected_num_frames) {
          std::stringstream err_stream;
          err_stream << "Failed to unwind sample " << sample_name << " properly.Expected "
                     << expected_num_frames << " frames, but unwinder contained "
                     << unwinder.NumFrames() << " frames. Unwind:\n"
                     << DumpFrames(unwinder);
          state.SkipWithError(err_stream.str().c_str());
          return;
        }
      }
      if (benchmarking_unwind) mem_tracker_.StopTrackingAllocations();
    };

    if (unwind_case_ == kSteadyState) {
      WarmUpUnwindCaches(offline_unwind_multiple_samples);
    }

    for (auto _ : state) {
      offline_unwind_multiple_samples(/*benchmarking_unwind=*/true);
    }
  }

  // This functions main purpose is to enable ELF caching for the steady state unwind case
  // and then perform one unwind to warm up the cache for subsequent unwinds.
  //
  // Another reason for pulling this functionality out of the main benchmarking function is
  // to add an additional call stack frame in between the cache warm-up unwinds and
  // BenchmarkOfflineUnwindMultipleSamples so that it is easy to filter this set of unwinds out
  // when profiling.
  void WarmUpUnwindCaches(const std::function<void(bool)>& offline_unwind_multiple_samples) {
    Elf::SetCachingEnabled(true);
    offline_unwind_multiple_samples(/*benchmarking_unwind=*/false);
  }

  std::string unwind_case_;
  bool resolve_names_;
  MemoryTracker mem_tracker_;
  OfflineUnwindUtils offline_utils_;
};

BENCHMARK_DEFINE_F(OfflineUnwindBenchmark, BM_offline_straddle_arm64)(benchmark::State& state) {
  SingleUnwindBenchmark(
      state, {.offline_files_dir = "straddle_arm64/", .arch = ARCH_ARM64, .create_maps = false});
}
BENCHMARK_REGISTER_F(OfflineUnwindBenchmark, BM_offline_straddle_arm64)
    ->ArgNames({"is_steady_state_case", "resolve_names"})
    ->Ranges({{false, true}, {false, true}});

BENCHMARK_DEFINE_F(OfflineUnwindBenchmark, BM_offline_straddle_arm64_cached_maps)
(benchmark::State& state) {
  SingleUnwindBenchmark(state, {.offline_files_dir = "straddle_arm64/", .arch = ARCH_ARM64});
}
BENCHMARK_REGISTER_F(OfflineUnwindBenchmark, BM_offline_straddle_arm64_cached_maps)
    ->ArgNames({"is_steady_state_case", "resolve_names"})
    ->Ranges({{false, true}, {false, true}});

BENCHMARK_DEFINE_F(OfflineUnwindBenchmark, BM_offline_jit_debug_arm)(benchmark::State& state) {
  SingleUnwindBenchmark(state, {.offline_files_dir = "jit_debug_arm/",
                                .arch = ARCH_ARM,
                                .memory_flag = ProcessMemoryFlag::kIncludeJitMemory,
                                .create_maps = false});
}
BENCHMARK_REGISTER_F(OfflineUnwindBenchmark, BM_offline_jit_debug_arm)
    ->ArgNames({"is_steady_state_case", "resolve_names"})
    ->Ranges({{false, true}, {false, true}});

BENCHMARK_DEFINE_F(OfflineUnwindBenchmark, BM_offline_profiler_like_multi_process)
(benchmark::State& state) {
  ConsecutiveUnwindBenchmark(
      state,
      std::vector<UnwindSampleInfo>{
          {.offline_files_dir = "bluetooth_arm64/pc_1/", .arch = ARCH_ARM64, .create_maps = false},
          {.offline_files_dir = "jit_debug_arm/",
           .arch = ARCH_ARM,
           .memory_flag = ProcessMemoryFlag::kIncludeJitMemory,
           .create_maps = false},
          {.offline_files_dir = "photos_reset_arm64/", .arch = ARCH_ARM64, .create_maps = false},
          {.offline_files_dir = "youtube_compiled_arm64/",
           .arch = ARCH_ARM64,
           .create_maps = false},
          {.offline_files_dir = "yt_music_arm64/", .arch = ARCH_ARM64, .create_maps = false},
          {.offline_files_dir = "maps_compiled_arm64/28656_oat_odex_jar/",
           .arch = ARCH_ARM64,
           .create_maps = false}});
}
BENCHMARK_REGISTER_F(OfflineUnwindBenchmark, BM_offline_profiler_like_multi_process)
    ->ArgNames({"is_steady_state_case", "resolve_names"})
    ->Ranges({{false, true}, {false, true}});

BENCHMARK_DEFINE_F(OfflineUnwindBenchmark, BM_offline_profiler_like_single_process_multi_thread)
(benchmark::State& state) {
  ConsecutiveUnwindBenchmark(
      state,
      std::vector<UnwindSampleInfo>{{.offline_files_dir = "maps_compiled_arm64/28656_oat_odex_jar/",
                                     .arch = ARCH_ARM64,
                                     .create_maps = false},
                                    {.offline_files_dir = "maps_compiled_arm64/28613_main-thread/",
                                     .arch = ARCH_ARM64,
                                     .create_maps = false},
                                    {.offline_files_dir = "maps_compiled_arm64/28644/",
                                     .arch = ARCH_ARM64,
                                     .create_maps = false},
                                    {.offline_files_dir = "maps_compiled_arm64/28648/",
                                     .arch = ARCH_ARM64,
                                     .create_maps = false},
                                    {.offline_files_dir = "maps_compiled_arm64/28667/",
                                     .arch = ARCH_ARM64,
                                     .create_maps = false}});
}
BENCHMARK_REGISTER_F(OfflineUnwindBenchmark, BM_offline_profiler_like_single_process_multi_thread)
    ->ArgNames({"is_steady_state_case", "resolve_names"})
    ->Ranges({{false, true}, {false, true}});

BENCHMARK_DEFINE_F(OfflineUnwindBenchmark, BM_offline_profiler_like_single_thread_diverse_pcs)
(benchmark::State& state) {
  ConsecutiveUnwindBenchmark(
      state,
      std::vector<UnwindSampleInfo>{
          {.offline_files_dir = "bluetooth_arm64/pc_1/", .arch = ARCH_ARM64, .create_maps = false},
          {.offline_files_dir = "bluetooth_arm64/pc_2/", .arch = ARCH_ARM64, .create_maps = false},
          {.offline_files_dir = "bluetooth_arm64/pc_3/", .arch = ARCH_ARM64, .create_maps = false},
          {.offline_files_dir = "bluetooth_arm64/pc_4/",
           .arch = ARCH_ARM64,
           .create_maps = false}});
}
BENCHMARK_REGISTER_F(OfflineUnwindBenchmark, BM_offline_profiler_like_single_thread_diverse_pcs)
    ->ArgNames({"is_steady_state_case", "resolve_names"})
    ->Ranges({{false, true}, {false, true}});

}  // namespace
}  // namespace unwindstack
