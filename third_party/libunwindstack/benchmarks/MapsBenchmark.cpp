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
#include <stdint.h>

#include <string>

#include <android-base/file.h>
#include <android-base/stringprintf.h>

#include <benchmark/benchmark.h>

#include <unwindstack/Maps.h>

class BenchmarkLocalUpdatableMaps : public unwindstack::LocalUpdatableMaps {
 public:
  BenchmarkLocalUpdatableMaps() : unwindstack::LocalUpdatableMaps() {}
  virtual ~BenchmarkLocalUpdatableMaps() = default;

  const std::string GetMapsFile() const override { return maps_file_; }

  void BenchmarkSetMapsFile(const std::string& maps_file) { maps_file_ = maps_file; }

 private:
  std::string maps_file_;
};

static constexpr size_t kNumSmallMaps = 100;
static constexpr size_t kNumLargeMaps = 10000;

static void CreateMap(const char* filename, size_t num_maps, size_t increment = 1) {
  std::string maps;
  for (size_t i = 0; i < num_maps; i += increment) {
    maps += android::base::StringPrintf("%zu-%zu r-xp 0000 00:00 0 name%zu\n", i * 1000,
                                        (i + 1) * 1000 * increment, i * increment);
  }
  if (!android::base::WriteStringToFile(maps, filename)) {
    errx(1, "WriteStringToFile failed");
  }
}

static void ReparseBenchmark(benchmark::State& state, const char* maps1, size_t maps1_total,
                             const char* maps2, size_t maps2_total) {
  for (auto _ : state) {
    BenchmarkLocalUpdatableMaps maps;
    maps.BenchmarkSetMapsFile(maps1);
    if (!maps.Reparse()) {
      errx(1, "Internal Error: reparse of initial maps filed.");
    }
    if (maps.Total() != maps1_total) {
      errx(1, "Internal Error: Incorrect total number of maps %zu, expected %zu.", maps.Total(),
           maps1_total);
    }
    maps.BenchmarkSetMapsFile(maps2);
    if (!maps.Reparse()) {
      errx(1, "Internal Error: reparse of second set of maps filed.");
    }
    if (maps.Total() != maps2_total) {
      errx(1, "Internal Error: Incorrect total number of maps %zu, expected %zu.", maps.Total(),
           maps2_total);
    }
  }
}

void BM_local_updatable_maps_reparse_double_initial_small(benchmark::State& state) {
  TemporaryFile initial_maps;
  CreateMap(initial_maps.path, kNumSmallMaps, 2);

  TemporaryFile reparse_maps;
  CreateMap(reparse_maps.path, kNumSmallMaps);

  ReparseBenchmark(state, initial_maps.path, kNumSmallMaps / 2, reparse_maps.path, kNumSmallMaps);
}
BENCHMARK(BM_local_updatable_maps_reparse_double_initial_small);

void BM_local_updatable_maps_reparse_double_initial_large(benchmark::State& state) {
  TemporaryFile initial_maps;
  CreateMap(initial_maps.path, kNumLargeMaps, 2);

  TemporaryFile reparse_maps;
  CreateMap(reparse_maps.path, kNumLargeMaps);

  ReparseBenchmark(state, initial_maps.path, kNumLargeMaps / 2, reparse_maps.path, kNumLargeMaps);
}
BENCHMARK(BM_local_updatable_maps_reparse_double_initial_large);

void BM_local_updatable_maps_reparse_same_maps_small(benchmark::State& state) {
  static constexpr size_t kNumSmallMaps = 100;
  TemporaryFile maps;
  CreateMap(maps.path, kNumSmallMaps);

  ReparseBenchmark(state, maps.path, kNumSmallMaps, maps.path, kNumSmallMaps);
}
BENCHMARK(BM_local_updatable_maps_reparse_same_maps_small);

void BM_local_updatable_maps_reparse_same_maps_large(benchmark::State& state) {
  TemporaryFile maps;
  CreateMap(maps.path, kNumLargeMaps);

  ReparseBenchmark(state, maps.path, kNumLargeMaps, maps.path, kNumLargeMaps);
}
BENCHMARK(BM_local_updatable_maps_reparse_same_maps_large);

void BM_local_updatable_maps_reparse_few_extra_small(benchmark::State& state) {
  TemporaryFile maps1;
  CreateMap(maps1.path, kNumSmallMaps - 4);

  TemporaryFile maps2;
  CreateMap(maps2.path, kNumSmallMaps);

  ReparseBenchmark(state, maps1.path, kNumSmallMaps - 4, maps2.path, kNumSmallMaps);
}
BENCHMARK(BM_local_updatable_maps_reparse_few_extra_small);

void BM_local_updatable_maps_reparse_few_extra_large(benchmark::State& state) {
  TemporaryFile maps1;
  CreateMap(maps1.path, kNumLargeMaps - 4);

  TemporaryFile maps2;
  CreateMap(maps2.path, kNumLargeMaps);

  ReparseBenchmark(state, maps1.path, kNumLargeMaps - 4, maps2.path, kNumLargeMaps);
}
BENCHMARK(BM_local_updatable_maps_reparse_few_extra_large);

void BM_local_updatable_maps_reparse_few_less_small(benchmark::State& state) {
  TemporaryFile maps1;
  CreateMap(maps1.path, kNumSmallMaps);

  TemporaryFile maps2;
  CreateMap(maps2.path, kNumSmallMaps - 4);

  ReparseBenchmark(state, maps1.path, kNumSmallMaps, maps2.path, kNumSmallMaps - 4);
}
BENCHMARK(BM_local_updatable_maps_reparse_few_less_small);

void BM_local_updatable_maps_reparse_few_less_large(benchmark::State& state) {
  TemporaryFile maps1;
  CreateMap(maps1.path, kNumLargeMaps);

  TemporaryFile maps2;
  CreateMap(maps2.path, kNumLargeMaps - 4);

  ReparseBenchmark(state, maps1.path, kNumLargeMaps, maps2.path, kNumLargeMaps - 4);
}
BENCHMARK(BM_local_updatable_maps_reparse_few_less_large);
