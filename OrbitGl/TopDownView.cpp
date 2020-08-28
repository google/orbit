// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TopDownView.h"

TopDownFunction* TopDownInternalNode::GetFunctionOrNull(uint64_t function_absolute_address) {
  auto top_down_function_it = function_nodes_.find(function_absolute_address);
  if (top_down_function_it == function_nodes_.end()) {
    return nullptr;
  }
  return &top_down_function_it->second;
}

TopDownFunction* TopDownInternalNode::AddAndGetFunction(uint64_t function_absolute_address,
                                                        std::string function_name) {
  function_nodes_.insert_or_assign(
      function_absolute_address,
      TopDownFunction{function_absolute_address, std::move(function_name), this});
  return &function_nodes_.at(function_absolute_address);
}

TopDownThread* TopDownView::GetThreadOrNull(int32_t thread_id) {
  auto top_down_thread_it = thread_nodes_.find(thread_id);
  if (top_down_thread_it == thread_nodes_.end()) {
    return nullptr;
  }
  return &top_down_thread_it->second;
}

TopDownThread* TopDownView::AddAndGetThread(int32_t thread_id, std::string thread_name) {
  thread_nodes_.insert_or_assign(thread_id, TopDownThread{thread_id, std::move(thread_name), this});
  return &thread_nodes_.at(thread_id);
}

[[nodiscard]] static TopDownFunction* GetOrCreateFunctionNode(
    TopDownInternalNode* current_thread_or_function, uint64_t frame,
    const std::string& function_name) {
  TopDownFunction* function_node = current_thread_or_function->GetFunctionOrNull(frame);
  if (function_node == nullptr) {
    std::string formatted_function_name;
    if (function_name != CaptureData::kUnknownFunctionOrModuleName) {
      formatted_function_name = function_name;
    } else {
      formatted_function_name = absl::StrFormat("[unknown@%#llx]", frame);
    }
    function_node =
        current_thread_or_function->AddAndGetFunction(frame, std::move(formatted_function_name));
  }
  return function_node;
}

static void AddCallstackToTopDownThread(TopDownThread* thread_node,
                                        const CallStack& resolved_callstack,
                                        uint64_t callstack_sample_count,
                                        const CaptureData& capture_data) {
  TopDownInternalNode* current_thread_or_function = thread_node;
  for (auto frame_it = resolved_callstack.GetFrames().crbegin();
       frame_it != resolved_callstack.GetFrames().crend(); ++frame_it) {
    uint64_t frame = *frame_it;
    const std::string& function_name = capture_data.GetFunctionNameByAddress(frame);
    TopDownFunction* function_node =
        GetOrCreateFunctionNode(current_thread_or_function, frame, function_name);
    function_node->IncreaseSampleCount(callstack_sample_count);
    current_thread_or_function = function_node;
  }
}

[[nodiscard]] static TopDownThread* GetOrCreateThreadNode(
    TopDownView* top_down_view, int32_t tid, const std::string& process_name,
    const absl::flat_hash_map<int32_t, std::string>& thread_names) {
  TopDownThread* thread_node = top_down_view->GetThreadOrNull(tid);
  if (thread_node == nullptr) {
    std::string thread_name;
    if (tid == SamplingProfiler::kAllThreadsFakeTid) {
      thread_name = process_name;
    } else if (auto thread_name_it = thread_names.find(tid); thread_name_it != thread_names.end()) {
      thread_name = thread_name_it->second;
    }
    thread_node = top_down_view->AddAndGetThread(tid, std::move(thread_name));
  }
  return thread_node;
}

std::unique_ptr<TopDownView> TopDownView::CreateFromCaptureData(const CaptureData& capture_data) {
  auto top_down_view = std::make_unique<TopDownView>();
  const std::string& process_name = capture_data.process_name();
  const absl::flat_hash_map<int32_t, std::string>& thread_names = capture_data.thread_names();
  const SamplingProfiler& sampling_profiler = capture_data.sampling_profiler();
  for (const ThreadSampleData& thread_sample_data : sampling_profiler.GetThreadSampleData()) {
    const int32_t tid = thread_sample_data.thread_id;
    TopDownThread* thread_node =
        GetOrCreateThreadNode(top_down_view.get(), tid, process_name, thread_names);

    for (const auto& callstack_id_and_count : thread_sample_data.callstack_count) {
      const CallStack& resolved_callstack =
          sampling_profiler.GetResolvedCallstack(callstack_id_and_count.first);
      const uint64_t sample_count = callstack_id_and_count.second;
      // Don't count samples from the all-thread case again.
      if (tid != SamplingProfiler::kAllThreadsFakeTid) {
        top_down_view->IncreaseSampleCount(sample_count);
      }
      thread_node->IncreaseSampleCount(sample_count);

      AddCallstackToTopDownThread(thread_node, resolved_callstack, sample_count, capture_data);
    }
  }
  return top_down_view;
}
