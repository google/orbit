// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/PostProcessedSamplingData.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stddef.h>

#include <algorithm>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"

namespace orbit_client_data {

namespace {

std::multimap<int, uint64_t> SortCallstacks(const ThreadSampleData& data,
                                            const std::set<uint64_t>& callstacks) {
  std::multimap<int, uint64_t> sorted_callstacks;
  for (uint64_t id : callstacks) {
    auto it = data.sampled_callstack_id_to_count.find(id);
    if (it != data.sampled_callstack_id_to_count.end()) {
      int count = it->second;
      sorted_callstacks.insert(std::make_pair(count, id));
    }
  }

  return sorted_callstacks;
}

}  // namespace

uint32_t ThreadSampleData::GetCountForAddress(uint64_t address) const {
  auto it = sampled_address_to_count.find(address);
  if (it == sampled_address_to_count.end()) {
    return 0;
  }
  return it->second;
}

const orbit_client_protos::CallstackInfo& PostProcessedSamplingData::GetResolvedCallstack(
    uint64_t sampled_callstack_id) const {
  auto resolved_callstack_id_it = original_id_to_resolved_callstack_id_.find(sampled_callstack_id);
  CHECK(resolved_callstack_id_it != original_id_to_resolved_callstack_id_.end());
  auto resolved_callstack_it = id_to_resolved_callstack_.find(resolved_callstack_id_it->second);
  CHECK(resolved_callstack_it != id_to_resolved_callstack_.end());
  return resolved_callstack_it->second;
}

std::multimap<int, uint64_t> PostProcessedSamplingData::GetCallstacksFromAddresses(
    const std::vector<uint64_t>& addresses, ThreadID thread_id) const {
  const auto& sample_data_it = thread_id_to_sample_data_.find(thread_id);
  if (sample_data_it == thread_id_to_sample_data_.end()) {
    return std::multimap<int, uint64_t>();
  }

  std::set<uint64_t> callstacks;
  for (uint64_t address : addresses) {
    const auto& callstacks_it = function_address_to_sampled_callstack_ids_.find(address);
    if (callstacks_it != function_address_to_sampled_callstack_ids_.end()) {
      callstacks.insert(callstacks_it->second.begin(), callstacks_it->second.end());
    }
  }

  if (callstacks.empty()) {
    return std::multimap<int, uint64_t>();
  }
  return SortCallstacks(sample_data_it->second, callstacks);
}

std::unique_ptr<SortedCallstackReport>
PostProcessedSamplingData::GetSortedCallstackReportFromAddresses(
    const std::vector<uint64_t>& addresses, ThreadID thread_id) const {
  std::unique_ptr<SortedCallstackReport> report = std::make_unique<SortedCallstackReport>();
  std::multimap<int, uint64_t> multi_map = GetCallstacksFromAddresses(addresses, thread_id);
  size_t unique_callstacks_count = multi_map.size();
  report->callstacks_count.resize(unique_callstacks_count);
  size_t index = unique_callstacks_count;

  for (const auto& pair : multi_map) {
    CallstackCount* callstack = &report->callstacks_count[--index];
    callstack->count = pair.first;
    callstack->callstack_id = pair.second;
    report->callstacks_total_count += callstack->count;
  }

  return report;
}

const ThreadSampleData* PostProcessedSamplingData::GetThreadSampleDataByThreadId(
    int32_t thread_id) const {
  auto it = thread_id_to_sample_data_.find(thread_id);
  if (it == thread_id_to_sample_data_.end()) {
    return nullptr;
  }

  return &it->second;
}

const ThreadSampleData* PostProcessedSamplingData::GetSummary() const {
  auto summary_it = thread_id_to_sample_data_.find(orbit_base::kAllProcessThreadsTid);
  if (summary_it == thread_id_to_sample_data_.end()) {
    return nullptr;
  }
  return &(summary_it->second);
}

uint32_t PostProcessedSamplingData::GetCountOfFunction(uint64_t function_address) const {
  const ThreadSampleData* summary = GetSummary();
  if (summary != nullptr) {
    auto count_it = summary->resolved_address_to_count.find(function_address);
    if (count_it != summary->resolved_address_to_count.end()) {
      return count_it->second;
    }
    return 0;
  }

  uint32_t result = 0;
  for (const auto& [tid, thread_sample_data] : thread_id_to_sample_data_) {
    CHECK(tid != orbit_base::kAllProcessThreadsTid);  // Because GetSummary() == nullptr

    auto count_it = thread_sample_data.resolved_address_to_count.find(function_address);
    if (count_it != thread_sample_data.resolved_address_to_count.end()) {
      result += count_it->second;
    }
  }
  return result;
}

}  // namespace orbit_client_data
