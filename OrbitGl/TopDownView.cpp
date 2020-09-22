// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TopDownView.h"

std::vector<const TopDownNode*> TopDownNode::children() const {
  std::vector<const TopDownNode*> ret;
  for (const auto& tid_and_thread : thread_children_) {
    ret.push_back(&tid_and_thread.second);
  }
  for (auto& address_and_functions : function_children_) {
    ret.push_back(&address_and_functions.second);
  }
  return ret;
}

TopDownThread* TopDownNode::GetThreadOrNull(int32_t thread_id) {
  auto top_down_thread_it = thread_children_.find(thread_id);
  if (top_down_thread_it == thread_children_.end()) {
    return nullptr;
  }
  return &top_down_thread_it->second;
}

TopDownThread* TopDownNode::AddAndGetThread(int32_t thread_id, std::string thread_name) {
  thread_children_.insert_or_assign(thread_id,
                                    TopDownThread{thread_id, std::move(thread_name), this});
  return &thread_children_.at(thread_id);
}

TopDownFunction* TopDownNode::GetFunctionOrNull(uint64_t function_absolute_address) {
  auto top_down_function_it = function_children_.find(function_absolute_address);
  if (top_down_function_it == function_children_.end()) {
    return nullptr;
  }
  return &top_down_function_it->second;
}

TopDownFunction* TopDownNode::AddAndGetFunction(uint64_t function_absolute_address,
                                                std::string function_name,
                                                std::string module_path) {
  function_children_.insert_or_assign(
      function_absolute_address,
      TopDownFunction{function_absolute_address, std::move(function_name), std::move(module_path),
                      this});
  return &function_children_.at(function_absolute_address);
}

uint64_t TopDownNode::GetExclusiveSampleCount() const {
  uint64_t children_sample_count = 0;
  for (const auto& address_and_function : function_children_) {
    children_sample_count += address_and_function.second.sample_count();
  }
  for (const auto& tid_and_thread : thread_children_) {
    children_sample_count += tid_and_thread.second.sample_count();
  }
  return sample_count() - children_sample_count;
}

[[nodiscard]] static TopDownFunction* GetOrCreateFunctionNode(
    TopDownNode* current_thread_or_function, uint64_t frame, const std::string& function_name,
    const std::string& module_path) {
  TopDownFunction* function_node = current_thread_or_function->GetFunctionOrNull(frame);
  if (function_node == nullptr) {
    std::string formatted_function_name;
    if (function_name != CaptureData::kUnknownFunctionOrModuleName) {
      formatted_function_name = function_name;
    } else {
      formatted_function_name = absl::StrFormat("[unknown@%#llx]", frame);
    }
    function_node = current_thread_or_function->AddAndGetFunction(
        frame, std::move(formatted_function_name), module_path);
  }
  return function_node;
}

static void AddCallstackToTopDownThread(TopDownThread* thread_node,
                                        const CallStack& resolved_callstack,
                                        uint64_t callstack_sample_count,
                                        const CaptureData& capture_data) {
  TopDownNode* current_thread_or_function = thread_node;
  for (auto frame_it = resolved_callstack.GetFrames().crbegin();
       frame_it != resolved_callstack.GetFrames().crend(); ++frame_it) {
    uint64_t frame = *frame_it;
    const std::string& function_name = capture_data.GetFunctionNameByAddress(frame);
    const std::string& module_path = capture_data.GetModulePathByAddress(frame);
    TopDownFunction* function_node =
        GetOrCreateFunctionNode(current_thread_or_function, frame, function_name, module_path);
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

std::unique_ptr<TopDownView> TopDownView::CreateFromSamplingProfiler(
    const SamplingProfiler& sampling_profiler, const CaptureData& capture_data) {
  auto top_down_view = std::make_unique<TopDownView>();
  const std::string& process_name = capture_data.process_name();
  const absl::flat_hash_map<int32_t, std::string>& thread_names = capture_data.thread_names();

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
