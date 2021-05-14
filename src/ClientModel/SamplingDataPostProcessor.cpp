// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientModel/SamplingDataPostProcessor.h"

#include <absl/meta/type_traits.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/Callstack.h"
#include "ClientData/CallstackTypes.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture_data.pb.h"

using orbit_client_data::CallStack;
using orbit_client_data::CallstackData;
using orbit_client_data::PostProcessedSamplingData;
using orbit_client_data::SampledFunction;
using orbit_client_data::ThreadID;
using orbit_client_data::ThreadSampleData;

using orbit_client_protos::CallstackEvent;

namespace orbit_client_model {

namespace {

class SamplingDataPostProcessor {
 public:
  explicit SamplingDataPostProcessor() = default;
  SamplingDataPostProcessor& operator=(const SamplingDataPostProcessor& other) = default;
  SamplingDataPostProcessor(const SamplingDataPostProcessor& other) = default;

  SamplingDataPostProcessor(SamplingDataPostProcessor&& other) = default;
  SamplingDataPostProcessor& operator=(SamplingDataPostProcessor&& other) = default;

  PostProcessedSamplingData ProcessSamples(const CallstackData& callstack_data,
                                           const CaptureData& capture_data, bool generate_summary);

 private:
  void SortByThreadUsage();

  void ResolveCallstacks(const CallstackData& callstack_data, const CaptureData& capture_data);

  void MapAddressToFunctionAddress(uint64_t absolute_address, const CaptureData& capture_data);

  void FillThreadSampleDataSampleReports(const CaptureData& capture_data);

  // Filled by ProcessSamples.
  absl::flat_hash_map<ThreadID, ThreadSampleData> thread_id_to_sample_data_;
  absl::flat_hash_map<uint64_t, CallStack> id_to_resolved_callstack_;
  absl::flat_hash_map<std::vector<uint64_t>, uint64_t> resolved_callstack_to_id_;
  absl::flat_hash_map<uint64_t, uint64_t> original_id_to_resolved_callstack_id_;
  absl::flat_hash_map<uint64_t, absl::flat_hash_set<uint64_t>>
      function_address_to_sampled_callstack_ids_;
  absl::flat_hash_map<uint64_t, uint64_t> exact_address_to_function_address_;
  absl::flat_hash_map<uint64_t, absl::flat_hash_set<uint64_t>> function_address_to_exact_addresses_;
  std::vector<ThreadSampleData> sorted_thread_sample_data_;
};

}  // namespace

PostProcessedSamplingData CreatePostProcessedSamplingData(const CallstackData& callstack_data,
                                                          const CaptureData& capture_data,
                                                          bool generate_summary) {
  return SamplingDataPostProcessor{}.ProcessSamples(callstack_data, capture_data, generate_summary);
}

namespace {
PostProcessedSamplingData SamplingDataPostProcessor::ProcessSamples(
    const CallstackData& callstack_data, const CaptureData& capture_data, bool generate_summary) {
  // Unique call stacks and per thread data
  callstack_data.ForEachCallstackEvent(
      [this, &callstack_data, generate_summary](const CallstackEvent& event) {
        CHECK(callstack_data.HasCallStack(event.callstack_id()));

        ThreadSampleData* thread_sample_data = &thread_id_to_sample_data_[event.thread_id()];
        thread_sample_data->samples_count++;
        thread_sample_data->sampled_callstack_id_to_count[event.callstack_id()]++;
        callstack_data.ForEachFrameInCallstack(
            event.callstack_id(), [&thread_sample_data](uint64_t address) {
              thread_sample_data->sampled_address_to_count[address]++;
            });

        if (generate_summary) {
          ThreadSampleData* all_thread_sample_data =
              &thread_id_to_sample_data_[orbit_base::kAllProcessThreadsTid];
          all_thread_sample_data->samples_count++;
          all_thread_sample_data->sampled_callstack_id_to_count[event.callstack_id()]++;
          callstack_data.ForEachFrameInCallstack(
              event.callstack_id(), [&all_thread_sample_data](uint64_t address) {
                all_thread_sample_data->sampled_address_to_count[address]++;
              });
        }
      });

  ResolveCallstacks(callstack_data, capture_data);

  for (auto& sample_data_it : thread_id_to_sample_data_) {
    ThreadSampleData* thread_sample_data = &sample_data_it.second;

    // Address count per sample per thread
    for (const auto& [sampled_callstack_id, callstack_count] :
         thread_sample_data->sampled_callstack_id_to_count) {
      uint64_t resolved_callstack_id = original_id_to_resolved_callstack_id_[sampled_callstack_id];
      const CallStack& resolved_callstack = id_to_resolved_callstack_[resolved_callstack_id];

      // exclusive stat
      thread_sample_data->resolved_address_to_exclusive_count[resolved_callstack.GetFrame(0)] +=
          callstack_count;

      absl::flat_hash_set<uint64_t> unique_resolved_addresses;
      for (uint64_t resolved_address : resolved_callstack.frames()) {
        unique_resolved_addresses.insert(resolved_address);
      }

      for (uint64_t resolved_address : unique_resolved_addresses) {
        thread_sample_data->resolved_address_to_count[resolved_address] += callstack_count;
      }
    }

    // sort thread addresses by count
    for (const auto& address_count_it : thread_sample_data->resolved_address_to_count) {
      const uint64_t address = address_count_it.first;
      const uint32_t count = address_count_it.second;
      thread_sample_data->sorted_count_to_resolved_address.insert(std::make_pair(count, address));
    }
  }

  FillThreadSampleDataSampleReports(capture_data);

  SortByThreadUsage();

  return PostProcessedSamplingData(
      std::move(thread_id_to_sample_data_), std::move(id_to_resolved_callstack_),
      std::move(original_id_to_resolved_callstack_id_),
      std::move(function_address_to_sampled_callstack_ids_),
      std::move(function_address_to_exact_addresses_), std::move(sorted_thread_sample_data_));
}

void SamplingDataPostProcessor::SortByThreadUsage() {
  sorted_thread_sample_data_.reserve(thread_id_to_sample_data_.size());

  for (auto& pair : thread_id_to_sample_data_) {
    ThreadSampleData& data = pair.second;
    data.thread_id = pair.first;
    sorted_thread_sample_data_.push_back(data);
  }

  sort(sorted_thread_sample_data_.begin(), sorted_thread_sample_data_.end(),
       [](const ThreadSampleData& a, const ThreadSampleData& b) {
         return a.samples_count > b.samples_count;
       });
}

void SamplingDataPostProcessor::ResolveCallstacks(const CallstackData& callstack_data,
                                                  const CaptureData& capture_data) {
  callstack_data.ForEachUniqueCallstack(
      [this, &capture_data](uint64_t callstack_id, const CallStack& call_stack) {
        // A "resolved callstack" is a callstack where every address is replaced
        // by the start address of the function (if known).
        std::vector<uint64_t> resolved_callstack_data;

        for (uint64_t address : call_stack.frames()) {
          if (exact_address_to_function_address_.find(address) ==
              exact_address_to_function_address_.end()) {
            MapAddressToFunctionAddress(address, capture_data);
          }
          uint64_t function_address = exact_address_to_function_address_.at(address);

          resolved_callstack_data.push_back(function_address);
          function_address_to_sampled_callstack_ids_[function_address].insert(callstack_id);
        }

        // Check if we already have this callstack
        uint64_t resolved_callstack_id;
        auto it = resolved_callstack_to_id_.find(resolved_callstack_data);
        if (it == resolved_callstack_to_id_.end()) {
          resolved_callstack_id = callstack_id;
          CHECK(!id_to_resolved_callstack_.contains(resolved_callstack_id));
          id_to_resolved_callstack_.insert_or_assign(
              resolved_callstack_id,
              CallStack{{resolved_callstack_data.begin(), resolved_callstack_data.end()}});
          resolved_callstack_to_id_.insert_or_assign(std::move(resolved_callstack_data),
                                                     resolved_callstack_id);
        } else {
          resolved_callstack_id = it->second;
        }

        original_id_to_resolved_callstack_id_[callstack_id] = resolved_callstack_id;
      });
}

void SamplingDataPostProcessor::MapAddressToFunctionAddress(uint64_t absolute_address,
                                                            const CaptureData& capture_data) {
  // SamplingDataPostProcessor relies heavily on the association between address and function
  // address held by exact_address_to_function_address_, otherwise each address is considered a
  // different function. We are storing this mapping for faster lookup.
  std::optional<uint64_t> absolute_function_address_option =
      capture_data.FindFunctionAbsoluteAddressByAddress(absolute_address);
  uint64_t absolute_function_address = absolute_function_address_option.value_or(absolute_address);

  exact_address_to_function_address_[absolute_address] = absolute_function_address;
  function_address_to_exact_addresses_[absolute_function_address].insert(absolute_address);
}

void SamplingDataPostProcessor::FillThreadSampleDataSampleReports(const CaptureData& capture_data) {
  for (auto& data : thread_id_to_sample_data_) {
    ThreadSampleData* thread_sample_data = &data.second;
    std::vector<SampledFunction>* sampled_functions = &thread_sample_data->sampled_functions;

    for (auto sorted_it = thread_sample_data->sorted_count_to_resolved_address.rbegin();
         sorted_it != thread_sample_data->sorted_count_to_resolved_address.rend(); ++sorted_it) {
      uint32_t num_occurrences = sorted_it->first;
      uint64_t absolute_address = sorted_it->second;
      float inclusive_percent = 100.f * num_occurrences / thread_sample_data->samples_count;

      SampledFunction function;
      function.name = capture_data.GetFunctionNameByAddress(absolute_address);
      function.inclusive = inclusive_percent;
      function.exclusive = 0.f;
      auto it = thread_sample_data->resolved_address_to_exclusive_count.find(absolute_address);
      if (it != thread_sample_data->resolved_address_to_exclusive_count.end()) {
        function.exclusive = 100.f * it->second / thread_sample_data->samples_count;
      }
      function.absolute_address = absolute_address;
      function.module_path = capture_data.GetModulePathByAddress(absolute_address);

      sampled_functions->push_back(function);
    }
  }
}

}  // namespace

}  // namespace orbit_client_model
