// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TopDownView.h"

TopDownFunction* TopDownInternalNode::GetFunctionOrNull(
    uint64_t function_absolute_address) {
  auto top_down_function_it = function_nodes_.find(function_absolute_address);
  if (top_down_function_it == function_nodes_.end()) {
    return nullptr;
  }
  return &top_down_function_it->second;
}

TopDownFunction* TopDownInternalNode::AddAndGetFunction(
    uint64_t function_absolute_address, std::string function_name) {
  function_nodes_.insert_or_assign(
      function_absolute_address,
      TopDownFunction{function_absolute_address, std::move(function_name),
                      this});
  return &function_nodes_.at(function_absolute_address);
}

TopDownThread* TopDownView::GetThreadOrNull(int32_t thread_id) {
  auto top_down_thread_it = thread_nodes_.find(thread_id);
  if (top_down_thread_it == thread_nodes_.end()) {
    return nullptr;
  }
  return &top_down_thread_it->second;
}

TopDownThread* TopDownView::AddAndGetThread(int32_t thread_id,
                                            std::string thread_name) {
  thread_nodes_.insert_or_assign(
      thread_id, TopDownThread{thread_id, std::move(thread_name), this});
  return &thread_nodes_.at(thread_id);
}

std::unique_ptr<TopDownView> TopDownView::CreateFromSamplingProfiler(
    const SamplingProfiler& sampling_profiler, const std::string& process_name,
    const std::unordered_map<int32_t, std::string>& thread_names,
    const std::unordered_map<uint64_t, std::string>& function_names) {
  auto top_down_view = std::make_unique<TopDownView>();
  for (const ThreadSampleData* thread_sample_data :
       sampling_profiler.GetThreadSampleData()) {
    const int32_t tid = thread_sample_data->m_TID;
    TopDownThread* thread_node = top_down_view->GetThreadOrNull(tid);
    if (thread_node == nullptr) {
      std::string thread_name;
      // Use the node with tid == 0 as the container for all threads, like in
      // SamplingProfiler, SamplingReport(DataView), TimeGraph.
      if (tid == 0) {
        thread_name = process_name;
      } else {
        auto thread_name_it = thread_names.find(tid);
        if (thread_name_it != thread_names.end()) {
          thread_name = thread_name_it->second;
        }
      }
      thread_node = top_down_view->AddAndGetThread(tid, std::move(thread_name));
    }

    for (const auto& callstack_id_and_count :
         thread_sample_data->m_CallstackCount) {
      std::shared_ptr<CallStack> resolved_callstack =
          sampling_profiler.GetResolvedCallstack(callstack_id_and_count.first);
      if (resolved_callstack == nullptr) {
        continue;
      }
      const uint64_t sample_count = callstack_id_and_count.second;
      // Don't count samples from the all-thread case (tid == 0) again.
      if (tid != 0) {
        top_down_view->IncreaseSampleCount(sample_count);
      }
      thread_node->IncreaseSampleCount(sample_count);

      TopDownInternalNode* current_thread_or_function = thread_node;
      for (auto frame_it = resolved_callstack->m_Data.crbegin();
           frame_it != resolved_callstack->m_Data.crend(); ++frame_it) {
        uint64_t frame = *frame_it;
        TopDownFunction* function_node =
            current_thread_or_function->GetFunctionOrNull(frame);
        if (function_node == nullptr) {
          std::string function_name;
          auto function_name_it = function_names.find(frame);
          if (function_name_it != function_names.end() &&
              function_name_it->second != "???") {
            function_name = function_name_it->second;
          } else {
            function_name = absl::StrFormat("[unknown@%#llx]", frame);
          }
          function_node = current_thread_or_function->AddAndGetFunction(
              frame, std::move(function_name));
        }
        function_node->IncreaseSampleCount(sample_count);
        current_thread_or_function = function_node;
      }
    }
  }
  return top_down_view;
}
