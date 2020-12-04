// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitClientModel/SamplingDataPostProcessor.h"

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/CallstackTypes.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture_data.pb.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;

namespace orbit_client_model::sampling_data_post_processor {

namespace {

class SamplingDataPostProcessor {
 public:
  explicit SamplingDataPostProcessor() = default;
  SamplingDataPostProcessor& operator=(const SamplingDataPostProcessor& other) = default;
  SamplingDataPostProcessor(const SamplingDataPostProcessor& other) = default;

  SamplingDataPostProcessor(SamplingDataPostProcessor&& other) = default;
  SamplingDataPostProcessor& operator=(SamplingDataPostProcessor&& other) = default;

  PostProcessedSamplingData ProcessSamples(const CallstackData& callstack_data,
                                           const CaptureData& capture_data, bool generate_summary) {
    // Unique call stacks and per thread data
    callstack_data.ForEachCallstackEvent(
        [this, &callstack_data, generate_summary](const CallstackEvent& event) {
          CHECK(callstack_data.HasCallStack(event.callstack_hash()));

          ThreadSampleData* thread_sample_data = &thread_id_to_sample_data_[event.thread_id()];
          thread_sample_data->samples_count++;
          thread_sample_data->callstack_count[event.callstack_hash()]++;
          callstack_data.ForEachFrameInCallstack(event.callstack_hash(),
                                                 [&thread_sample_data](uint64_t address) {
                                                   thread_sample_data->raw_address_count[address]++;
                                                 });

          if (generate_summary) {
            ThreadSampleData* all_thread_sample_data =
                &thread_id_to_sample_data_[orbit_base::kAllProcessThreadsFakeTid];
            all_thread_sample_data->samples_count++;
            all_thread_sample_data->callstack_count[event.callstack_hash()]++;
            callstack_data.ForEachFrameInCallstack(
                event.callstack_hash(), [&all_thread_sample_data](uint64_t address) {
                  all_thread_sample_data->raw_address_count[address]++;
                });
          }
        });

    ResolveCallstacks(callstack_data, capture_data);

    for (auto& sample_data_it : thread_id_to_sample_data_) {
      ThreadSampleData* thread_sample_data = &sample_data_it.second;

      // Address count per sample per thread
      for (const auto& callstack_count_it : thread_sample_data->callstack_count) {
        const CallstackID callstack_id = callstack_count_it.first;
        const uint32_t callstack_count = callstack_count_it.second;

        CallstackID resolved_callstack_id = original_to_resolved_callstack_[callstack_id];
        std::shared_ptr<CallStack>& resolved_callstack =
            unique_resolved_callstacks_[resolved_callstack_id];

        // exclusive stat
        thread_sample_data->exclusive_count[resolved_callstack->GetFrame(0)] += callstack_count;

        std::set<uint64_t> unique_addresses;
        for (uint64_t address : resolved_callstack->GetFrames()) {
          unique_addresses.insert(address);
        }

        for (uint64_t address : unique_addresses) {
          thread_sample_data->address_count[address] += callstack_count;
        }
      }

      // sort thread addresses by count
      for (const auto& address_count_it : thread_sample_data->address_count) {
        const uint64_t address = address_count_it.first;
        const uint32_t count = address_count_it.second;
        thread_sample_data->address_count_sorted.insert(std::make_pair(count, address));
      }
    }

    FillThreadSampleDataSampleReports(capture_data);

    SortByThreadUsage();

    return PostProcessedSamplingData(
        std::move(thread_id_to_sample_data_), std::move(unique_resolved_callstacks_),
        std::move(original_to_resolved_callstack_), std::move(function_address_to_callstack_),
        std::move(function_address_to_exact_addresses_), std::move(sorted_thread_sample_data_));
  }

 private:
  void SortByThreadUsage() {
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

  void ResolveCallstacks(const CallstackData& callstack_data, const CaptureData& capture_data) {
    callstack_data.ForEachUniqueCallstack([this, &capture_data](const CallStack& call_stack) {
      // A "resolved callstack" is a callstack where every address is replaced
      // by the start address of the function (if known).
      std::vector<uint64_t> resolved_callstack_data;

      for (uint64_t address : call_stack.GetFrames()) {
        if (exact_address_to_function_address_.find(address) ==
            exact_address_to_function_address_.end()) {
          MapAddressToFunctionAddress(address, capture_data);
        }
        uint64_t function_address = exact_address_to_function_address_.at(address);

        resolved_callstack_data.push_back(function_address);
        function_address_to_callstack_[function_address].insert(call_stack.GetHash());
      }

      CallStack resolved_callstack(std::move(resolved_callstack_data));

      CallstackID resolved_callstack_id = resolved_callstack.GetHash();
      if (unique_resolved_callstacks_.find(resolved_callstack_id) ==
          unique_resolved_callstacks_.end()) {
        unique_resolved_callstacks_[resolved_callstack_id] =
            std::make_shared<CallStack>(resolved_callstack);
      }

      original_to_resolved_callstack_[call_stack.GetHash()] = resolved_callstack_id;
    });
  }

  void MapAddressToFunctionAddress(uint64_t absolute_address, const CaptureData& capture_data) {
    const LinuxAddressInfo* address_info = capture_data.GetAddressInfo(absolute_address);
    const FunctionInfo* function = capture_data.FindFunctionByAddress(absolute_address, false);

    // Find the start address of the function this address falls inside.
    // Use the Function returned by Process::GetFunctionFromAddress, and
    // when this fails (e.g., the module containing the function has not
    // been loaded) use (for now) the LinuxAddressInfo that is collected
    // for every address in a callstack. SamplingProfiler relies heavily
    // on the association between address and function address held by
    // exact_address_to_function_address_, otherwise each address is
    // considered a different function.
    uint64_t absolute_function_address;
    if (function != nullptr) {
      absolute_function_address = capture_data.GetAbsoluteAddress(*function);
    } else if (address_info != nullptr) {
      absolute_function_address = absolute_address - address_info->offset_in_function();
    } else {
      absolute_function_address = absolute_address;
    }

    exact_address_to_function_address_[absolute_address] = absolute_function_address;
    function_address_to_exact_addresses_[absolute_function_address].insert(absolute_address);
  }

  void FillThreadSampleDataSampleReports(const CaptureData& capture_data) {
    for (auto& data : thread_id_to_sample_data_) {
      ThreadSampleData* thread_sample_data = &data.second;
      std::vector<SampledFunction>* sampled_functions = &thread_sample_data->sampled_function;

      for (auto sorted_it = thread_sample_data->address_count_sorted.rbegin();
           sorted_it != thread_sample_data->address_count_sorted.rend(); ++sorted_it) {
        uint32_t num_occurences = sorted_it->first;
        uint64_t absolute_address = sorted_it->second;
        float inclusive_percent = 100.f * num_occurences / thread_sample_data->samples_count;

        SampledFunction function;
        function.name = capture_data.GetFunctionNameByAddress(absolute_address);
        function.inclusive = inclusive_percent;
        function.exclusive = 0.f;
        auto it = thread_sample_data->exclusive_count.find(absolute_address);
        if (it != thread_sample_data->exclusive_count.end()) {
          function.exclusive = 100.f * it->second / thread_sample_data->samples_count;
        }
        function.absolute_address = absolute_address;
        function.module_path = capture_data.GetModulePathByAddress(absolute_address);

        const FunctionInfo* function_info =
            capture_data.FindFunctionByAddress(absolute_address, false);
        if (function_info != nullptr) {
          function.line = function_info->line();
          function.file = function_info->file();
        }

        sampled_functions->push_back(function);
      }
    }
  }

  // Filled by ProcessSamples.
  absl::flat_hash_map<ThreadID, ThreadSampleData> thread_id_to_sample_data_;
  absl::flat_hash_map<CallstackID, std::shared_ptr<CallStack>> unique_resolved_callstacks_;
  absl::flat_hash_map<CallstackID, CallstackID> original_to_resolved_callstack_;
  absl::flat_hash_map<uint64_t, std::set<CallstackID>> function_address_to_callstack_;
  absl::flat_hash_map<uint64_t, uint64_t> exact_address_to_function_address_;
  absl::flat_hash_map<uint64_t, absl::flat_hash_set<uint64_t>> function_address_to_exact_addresses_;
  std::vector<ThreadSampleData> sorted_thread_sample_data_;
};

}  // namespace

PostProcessedSamplingData CreatePostProcessedSamplingData(const CallstackData& callstack_data,
                                                          const CaptureData& capture_data,
                                                          bool generate_summary) {
  SamplingDataPostProcessor profiler;
  return profiler.ProcessSamples(callstack_data, capture_data, generate_summary);
}

}  // namespace orbit_client_model::sampling_data_post_processor
