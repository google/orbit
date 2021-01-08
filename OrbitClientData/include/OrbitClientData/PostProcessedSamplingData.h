// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_POST_PROCESSED_SAMPLING_DATA_H_
#define ORBIT_CLIENT_DATA_POST_PROCESSED_SAMPLING_DATA_H_

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/CallstackTypes.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture_data.pb.h"

struct SampledFunction {
  SampledFunction() = default;

  std::string name;
  std::string module_path;
  std::string file;
  float exclusive = 0;
  float inclusive = 0;
  int line = 0;
  uint64_t absolute_address = 0;
  const orbit_client_protos::FunctionInfo* function = nullptr;
};

struct ThreadSampleData {
  ThreadSampleData() = default;
  [[nodiscard]] uint32_t GetCountForAddress(uint64_t address) const;
  absl::flat_hash_map<CallstackID, uint32_t> callstack_count;
  absl::flat_hash_map<uint64_t, uint32_t> address_count;
  absl::flat_hash_map<uint64_t, uint32_t> raw_address_count;
  absl::flat_hash_map<uint64_t, uint32_t> exclusive_count;
  std::multimap<uint32_t, uint64_t> address_count_sorted;
  uint32_t samples_count = 0;
  std::vector<SampledFunction> sampled_function;
  ThreadID thread_id = 0;
};

struct CallstackCount {
  CallstackCount() = default;

  int count = 0;
  CallstackID callstack_id = 0;
};

struct SortedCallstackReport {
  SortedCallstackReport() = default;
  int callstacks_total_count = 0;
  std::vector<CallstackCount> callstacks_count;
};

class PostProcessedSamplingData {
 public:
  PostProcessedSamplingData() = default;
  PostProcessedSamplingData(
      absl::flat_hash_map<ThreadID, ThreadSampleData> thread_id_to_sample_data,
      absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_resolved_callstacks,
      absl::flat_hash_map<CallstackID, CallstackID> original_to_resolved_callstack,
      absl::flat_hash_map<uint64_t, std::set<CallstackID>> function_address_to_callstack,
      absl::flat_hash_map<uint64_t, absl::flat_hash_set<uint64_t>>
          function_address_to_exact_addresses,
      std::vector<ThreadSampleData> sorted_thread_sample_data)
      : thread_id_to_sample_data_{std::move(thread_id_to_sample_data)},
        unique_resolved_callstacks_{std::move(unique_resolved_callstacks)},
        original_to_resolved_callstack_{std::move(original_to_resolved_callstack)},
        function_address_to_callstack_{std::move(function_address_to_callstack)},
        function_address_to_exact_addresses_{std::move(function_address_to_exact_addresses)},
        sorted_thread_sample_data_{std::move(sorted_thread_sample_data)} {};
  ~PostProcessedSamplingData() = default;
  PostProcessedSamplingData(const PostProcessedSamplingData& other) = default;
  PostProcessedSamplingData(PostProcessedSamplingData&& other) = default;
  PostProcessedSamplingData& operator=(const PostProcessedSamplingData& other) = default;

  PostProcessedSamplingData& operator=(PostProcessedSamplingData&& other) = default;

  [[nodiscard]] const CallStack& GetResolvedCallstack(CallstackID raw_callstack_id) const;

  [[nodiscard]] std::multimap<int, CallstackID> GetCallstacksFromAddress(uint64_t address,
                                                                         ThreadID thread_id) const;
  [[nodiscard]] std::unique_ptr<SortedCallstackReport> GetSortedCallstackReportFromAddress(
      uint64_t address, ThreadID thread_id) const;

  [[nodiscard]] const std::vector<ThreadSampleData>& GetThreadSampleData() const {
    return sorted_thread_sample_data_;
  }
  [[nodiscard]] const ThreadSampleData* GetThreadSampleDataByThreadId(int32_t thread_id) const {
    auto it = thread_id_to_sample_data_.find(thread_id);
    if (it == thread_id_to_sample_data_.end()) {
      return nullptr;
    }

    return &it->second;
  }

  [[nodiscard]] const ThreadSampleData* GetSummary() const;
  [[nodiscard]] uint32_t GetCountOfFunction(uint64_t function_address) const;

 private:
  absl::flat_hash_map<ThreadID, ThreadSampleData> thread_id_to_sample_data_;
  absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_resolved_callstacks_;
  absl::flat_hash_map<CallstackID, CallstackID> original_to_resolved_callstack_;
  absl::flat_hash_map<uint64_t, std::set<CallstackID>> function_address_to_callstack_;
  absl::flat_hash_map<uint64_t, absl::flat_hash_set<uint64_t>> function_address_to_exact_addresses_;
  std::vector<ThreadSampleData> sorted_thread_sample_data_;
};

#endif  // ORBIT_CLIENT_DATA_POST_PROCESSED_SAMPLING_DATA_H_
