// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallTreeView.h"

std::vector<const CallTreeNode*> CallTreeNode::children() const {
  std::vector<const CallTreeNode*> ret;
  for (const auto& tid_and_thread : thread_children_) {
    ret.push_back(&tid_and_thread.second);
  }
  for (auto& address_and_functions : function_children_) {
    ret.push_back(&address_and_functions.second);
  }
  return ret;
}

CallTreeThread* CallTreeNode::GetThreadOrNull(int32_t thread_id) {
  auto thread_it = thread_children_.find(thread_id);
  if (thread_it == thread_children_.end()) {
    return nullptr;
  }
  return &thread_it->second;
}

CallTreeThread* CallTreeNode::AddAndGetThread(int32_t thread_id, std::string thread_name) {
  thread_children_.insert_or_assign(thread_id,
                                    CallTreeThread{thread_id, std::move(thread_name), this});
  return &thread_children_.at(thread_id);
}

CallTreeFunction* CallTreeNode::GetFunctionOrNull(uint64_t function_absolute_address) {
  auto function_it = function_children_.find(function_absolute_address);
  if (function_it == function_children_.end()) {
    return nullptr;
  }
  return &function_it->second;
}

CallTreeFunction* CallTreeNode::AddAndGetFunction(uint64_t function_absolute_address,
                                                  std::string function_name,
                                                  std::string module_path) {
  function_children_.insert_or_assign(
      function_absolute_address,
      CallTreeFunction{function_absolute_address, std::move(function_name), std::move(module_path),
                       this});
  return &function_children_.at(function_absolute_address);
}

uint64_t CallTreeNode::GetExclusiveSampleCount() const {
  uint64_t children_sample_count = 0;
  for (const auto& address_and_function : function_children_) {
    children_sample_count += address_and_function.second.sample_count();
  }
  for (const auto& tid_and_thread : thread_children_) {
    children_sample_count += tid_and_thread.second.sample_count();
  }
  return sample_count() - children_sample_count;
}

[[nodiscard]] static CallTreeFunction* GetOrCreateFunctionNode(CallTreeNode* current_node,
                                                               uint64_t frame,
                                                               const std::string& function_name,
                                                               const std::string& module_path) {
  CallTreeFunction* function_node = current_node->GetFunctionOrNull(frame);
  if (function_node == nullptr) {
    std::string formatted_function_name;
    if (function_name != CaptureData::kUnknownFunctionOrModuleName) {
      formatted_function_name = function_name;
    } else {
      formatted_function_name = absl::StrFormat("[unknown@%#llx]", frame);
    }
    function_node =
        current_node->AddAndGetFunction(frame, std::move(formatted_function_name), module_path);
  }
  return function_node;
}

static void AddCallstackToTopDownThread(CallTreeThread* thread_node,
                                        const CallStack& resolved_callstack,
                                        uint64_t callstack_sample_count,
                                        const CaptureData& capture_data) {
  CallTreeNode* current_thread_or_function = thread_node;
  for (auto frame_it = resolved_callstack.GetFrames().crbegin();
       frame_it != resolved_callstack.GetFrames().crend(); ++frame_it) {
    uint64_t frame = *frame_it;
    const std::string& function_name = capture_data.GetFunctionNameByAddress(frame);
    const std::string& module_path = capture_data.GetModulePathByAddress(frame);
    CallTreeFunction* function_node =
        GetOrCreateFunctionNode(current_thread_or_function, frame, function_name, module_path);
    function_node->IncreaseSampleCount(callstack_sample_count);
    current_thread_or_function = function_node;
  }
}

[[nodiscard]] static CallTreeThread* GetOrCreateThreadNode(
    CallTreeNode* current_node, int32_t tid, const std::string& process_name,
    const absl::flat_hash_map<int32_t, std::string>& thread_names) {
  CallTreeThread* thread_node = current_node->GetThreadOrNull(tid);
  if (thread_node == nullptr) {
    std::string thread_name;
    if (tid == PostProcessedSamplingData::kAllThreadsFakeTid) {
      thread_name = process_name;
    } else if (auto thread_name_it = thread_names.find(tid); thread_name_it != thread_names.end()) {
      thread_name = thread_name_it->second;
    }
    thread_node = current_node->AddAndGetThread(tid, std::move(thread_name));
  }
  return thread_node;
}

std::unique_ptr<CallTreeView> CallTreeView::CreateTopDownViewFromSamplingProfiler(
    const PostProcessedSamplingData& post_processed_sampling_data,
    const CaptureData& capture_data) {
  auto top_down_view = std::make_unique<CallTreeView>();
  const std::string& process_name = capture_data.process_name();
  const absl::flat_hash_map<int32_t, std::string>& thread_names = capture_data.thread_names();

  for (const ThreadSampleData& thread_sample_data :
       post_processed_sampling_data.GetThreadSampleData()) {
    const int32_t tid = thread_sample_data.thread_id;
    CallTreeThread* thread_node =
        GetOrCreateThreadNode(top_down_view.get(), tid, process_name, thread_names);

    for (const auto& callstack_id_and_count : thread_sample_data.callstack_count) {
      const CallStack& resolved_callstack =
          post_processed_sampling_data.GetResolvedCallstack(callstack_id_and_count.first);
      const uint64_t sample_count = callstack_id_and_count.second;
      // Don't count samples from the all-thread case again.
      if (tid != PostProcessedSamplingData::kAllThreadsFakeTid) {
        top_down_view->IncreaseSampleCount(sample_count);
      }
      thread_node->IncreaseSampleCount(sample_count);

      AddCallstackToTopDownThread(thread_node, resolved_callstack, sample_count, capture_data);
    }
  }
  return top_down_view;
}

[[nodiscard]] static CallTreeNode* AddReversedCallstackToBottomUpViewAndReturnLastFunction(
    CallTreeView* bottom_up_view, const CallStack& resolved_callstack,
    uint64_t callstack_sample_count, const CaptureData& capture_data) {
  CallTreeNode* current_node = bottom_up_view;
  for (uint64_t frame : resolved_callstack.GetFrames()) {
    const std::string& function_name = capture_data.GetFunctionNameByAddress(frame);
    const std::string& module_path = capture_data.GetModulePathByAddress(frame);
    CallTreeFunction* function_node =
        GetOrCreateFunctionNode(current_node, frame, function_name, module_path);
    function_node->IncreaseSampleCount(callstack_sample_count);
    current_node = function_node;
  }
  return current_node;
}

std::unique_ptr<CallTreeView> CallTreeView::CreateBottomUpViewFromSamplingProfiler(
    const PostProcessedSamplingData& post_processed_sampling_data,
    const CaptureData& capture_data) {
  auto bottom_up_view = std::make_unique<CallTreeView>();
  const std::string& process_name = capture_data.process_name();
  const absl::flat_hash_map<int32_t, std::string>& thread_names = capture_data.thread_names();

  for (const ThreadSampleData& thread_sample_data :
       post_processed_sampling_data.GetThreadSampleData()) {
    const int32_t tid = thread_sample_data.thread_id;
    if (tid == PostProcessedSamplingData::kAllThreadsFakeTid) {
      continue;
    }

    for (const auto& callstack_id_and_count : thread_sample_data.callstack_count) {
      const CallStack& resolved_callstack =
          post_processed_sampling_data.GetResolvedCallstack(callstack_id_and_count.first);
      const uint64_t sample_count = callstack_id_and_count.second;
      bottom_up_view->IncreaseSampleCount(sample_count);

      CallTreeNode* last_node = AddReversedCallstackToBottomUpViewAndReturnLastFunction(
          bottom_up_view.get(), resolved_callstack, sample_count, capture_data);
      CallTreeThread* thread_node =
          GetOrCreateThreadNode(last_node, tid, process_name, thread_names);
      thread_node->IncreaseSampleCount(sample_count);
    }
  }

  return bottom_up_view;
}
