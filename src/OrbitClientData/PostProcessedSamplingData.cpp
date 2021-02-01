// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientData/PostProcessedSamplingData.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stddef.h>

#include <algorithm>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"

namespace {

std::multimap<int, CallstackID> SortCallstacks(const ThreadSampleData& data,
                                               const std::set<CallstackID>& callstacks) {
  std::multimap<int, CallstackID> sorted_callstacks;
  for (CallstackID id : callstacks) {
    auto it = data.callstack_count.find(id);
    if (it != data.callstack_count.end()) {
      int count = it->second;
      sorted_callstacks.insert(std::make_pair(count, id));
    }
  }

  return sorted_callstacks;
}

}  // namespace

uint32_t ThreadSampleData::GetCountForAddress(uint64_t address) const {
  auto res = raw_address_count.find(address);
  if (res == raw_address_count.end()) {
    return 0;
  }
  return (*res).second;
}

const CallStack& PostProcessedSamplingData::GetResolvedCallstack(
    CallstackID raw_callstack_id) const {
  auto resolved_callstack_id_it = original_to_resolved_callstack_.find(raw_callstack_id);
  CHECK(resolved_callstack_id_it != original_to_resolved_callstack_.end());
  auto resolved_callstack_it = unique_resolved_callstacks_.find(resolved_callstack_id_it->second);
  CHECK(resolved_callstack_it != unique_resolved_callstacks_.end());
  return resolved_callstack_it->second;
}

std::multimap<int, CallstackID> PostProcessedSamplingData::GetCallstacksFromAddresses(
    const std::vector<uint64_t>& addresses, ThreadID thread_id) const {
  const auto& sample_data_it = thread_id_to_sample_data_.find(thread_id);
  if (sample_data_it == thread_id_to_sample_data_.end()) {
    return std::multimap<int, CallstackID>();
  }

  std::set<CallstackID> callstacks;
  for (uint64_t address : addresses) {
    const auto& callstacks_it = function_address_to_callstack_.find(address);
    if (callstacks_it != function_address_to_callstack_.end()) {
      callstacks.insert(callstacks_it->second.begin(), callstacks_it->second.end());
    }
  }

  if (callstacks.empty()) {
    return std::multimap<int, CallstackID>();
  } else {
    return SortCallstacks(sample_data_it->second, callstacks);
  }
}

std::unique_ptr<SortedCallstackReport>
PostProcessedSamplingData::GetSortedCallstackReportFromAddresses(
    const std::vector<uint64_t>& addresses, ThreadID thread_id) const {
  std::unique_ptr<SortedCallstackReport> report = std::make_unique<SortedCallstackReport>();
  std::multimap<int, CallstackID> multi_map = GetCallstacksFromAddresses(addresses, thread_id);
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

const ThreadSampleData* PostProcessedSamplingData::GetSummary() const {
  auto summary_it = thread_id_to_sample_data_.find(orbit_base::kAllProcessThreadsTid);
  if (summary_it == thread_id_to_sample_data_.end()) {
    return nullptr;
  }
  return &(summary_it->second);
}

uint32_t PostProcessedSamplingData::GetCountOfFunction(uint64_t function_address) const {
  auto addresses_of_functions_itr = function_address_to_exact_addresses_.find(function_address);
  if (addresses_of_functions_itr == function_address_to_exact_addresses_.end()) {
    return 0;
  }
  uint32_t result = 0;
  const ThreadSampleData* summary = GetSummary();
  if (summary == nullptr) {
    return 0;
  }
  const auto& function_addresses = addresses_of_functions_itr->second;
  for (uint64_t address : function_addresses) {
    auto count_itr = summary->raw_address_count.find(address);
    if (count_itr != summary->raw_address_count.end()) {
      result += count_itr->second;
    }
  }
  return result;
}
