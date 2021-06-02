// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallTreeView.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>
#include <absl/meta/type_traits.h>
#include <absl/strings/str_format.h>

#include <algorithm>

#include "OrbitBase/ThreadConstants.h"
#include "capture_data.pb.h"

using orbit_client_data::PostProcessedSamplingData;
using orbit_client_data::ThreadSampleData;

using orbit_client_model::CaptureData;

using orbit_client_protos::CallstackInfo;

std::vector<const CallTreeNode*> CallTreeNode::children() const {
  std::vector<const CallTreeNode*> children;
  for (const auto& tid_and_thread : thread_children_) {
    children.push_back(&tid_and_thread.second);
  }
  for (const auto& address_and_functions : function_children_) {
    children.push_back(&address_and_functions.second);
  }
  if (unwind_errors_child_ != nullptr) {
    children.push_back(unwind_errors_child_.get());
  }
  return children;
}

CallTreeThread* CallTreeNode::GetThreadOrNull(int32_t thread_id) {
  auto thread_it = thread_children_.find(thread_id);
  if (thread_it == thread_children_.end()) {
    return nullptr;
  }
  return &thread_it->second;
}

CallTreeThread* CallTreeNode::AddAndGetThread(int32_t thread_id, std::string thread_name) {
  const auto& [it, inserted] = thread_children_.try_emplace(
      thread_id, CallTreeThread{thread_id, std::move(thread_name), this});
  CHECK(inserted);
  return &it->second;
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
                                                  std::string module_path,
                                                  std::string module_build_id) {
  const auto& [it, inserted] = function_children_.try_emplace(
      function_absolute_address,
      CallTreeFunction{function_absolute_address, std::move(function_name), std::move(module_path),
                       std::move(module_build_id), this});
  CHECK(inserted);
  return &it->second;
}

CallTreeUnwindErrors* CallTreeNode::GetUnwindErrorsOrNull() { return unwind_errors_child_.get(); }

CallTreeUnwindErrors* CallTreeNode::AddAndGetUnwindErrors() {
  CHECK(unwind_errors_child_ == nullptr);
  unwind_errors_child_ = std::make_unique<CallTreeUnwindErrors>(this);
  return unwind_errors_child_.get();
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
                                                               const std::string& module_path,
                                                               const std::string& module_build_id) {
  CallTreeFunction* function_node = current_node->GetFunctionOrNull(frame);
  if (function_node == nullptr) {
    std::string formatted_function_name;
    if (function_name != CaptureData::kUnknownFunctionOrModuleName) {
      formatted_function_name = function_name;
    } else {
      formatted_function_name = absl::StrFormat("[unknown@%#llx]", frame);
    }
    function_node = current_node->AddAndGetFunction(frame, std::move(formatted_function_name),
                                                    module_path, module_build_id);
  }
  return function_node;
}

static void AddCallstackToTopDownThread(CallTreeThread* thread_node,
                                        const CallstackInfo& resolved_callstack,
                                        uint64_t callstack_sample_count,
                                        const CaptureData& capture_data) {
  CallTreeNode* current_thread_or_function = thread_node;
  for (auto frame_it = resolved_callstack.frames().rbegin();
       frame_it != resolved_callstack.frames().rend(); ++frame_it) {
    uint64_t frame = *frame_it;
    const std::string& function_name = capture_data.GetFunctionNameByAddress(frame);
    const std::string& module_path = capture_data.GetModulePathByAddress(frame);
    const std::string module_build_id = capture_data.FindModuleBuildIdByAddress(frame).value_or("");

    CallTreeFunction* function_node = GetOrCreateFunctionNode(
        current_thread_or_function, frame, function_name, module_path, module_build_id);
    function_node->IncreaseSampleCount(callstack_sample_count);
    current_thread_or_function = function_node;
  }
}

static void AddUnwindErrorToTopDownThread(CallTreeThread* thread_node,
                                          const CallstackInfo& resolved_callstack,
                                          uint64_t callstack_sample_count,
                                          const CaptureData& capture_data) {
  CallTreeUnwindErrors* unwind_errors_node = thread_node->GetUnwindErrorsOrNull();
  if (unwind_errors_node == nullptr) {
    unwind_errors_node = thread_node->AddAndGetUnwindErrors();
  }
  unwind_errors_node->IncreaseSampleCount(callstack_sample_count);

  CHECK(!resolved_callstack.frames().empty());
  // Only use the innermost frame for unwind errors.
  uint64_t frame = resolved_callstack.frames(0);
  const std::string& function_name = capture_data.GetFunctionNameByAddress(frame);
  const std::string& module_path = capture_data.GetModulePathByAddress(frame);
  const std::string module_build_id = capture_data.FindModuleBuildIdByAddress(frame).value_or("");
  CallTreeFunction* function_node = GetOrCreateFunctionNode(
      unwind_errors_node, frame, function_name, module_path, module_build_id);
  function_node->IncreaseSampleCount(callstack_sample_count);
}

[[nodiscard]] static CallTreeThread* GetOrCreateThreadNode(
    CallTreeNode* current_node, int32_t tid, const std::string& process_name,
    const absl::flat_hash_map<int32_t, std::string>& thread_names) {
  CallTreeThread* thread_node = current_node->GetThreadOrNull(tid);
  if (thread_node == nullptr) {
    std::string thread_name;
    if (tid == orbit_base::kAllProcessThreadsTid) {
      thread_name = process_name;
    } else if (auto thread_name_it = thread_names.find(tid); thread_name_it != thread_names.end()) {
      thread_name = thread_name_it->second;
    }
    thread_node = current_node->AddAndGetThread(tid, std::move(thread_name));
  }
  return thread_node;
}

std::unique_ptr<CallTreeView> CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(
    const PostProcessedSamplingData& post_processed_sampling_data,
    const CaptureData& capture_data) {
  auto top_down_view = std::make_unique<CallTreeView>();
  const std::string& process_name = capture_data.process_name();
  const absl::flat_hash_map<int32_t, std::string>& thread_names = capture_data.thread_names();

  for (const ThreadSampleData& thread_sample_data :
       post_processed_sampling_data.GetThreadSampleData()) {
    const int32_t tid = thread_sample_data.thread_id;

    for (const auto& [callstack_id, sample_count] :
         thread_sample_data.sampled_callstack_id_to_count) {
      const CallstackInfo& resolved_callstack =
          post_processed_sampling_data.GetResolvedCallstack(callstack_id);

      // Don't count samples from the all-thread case again.
      if (tid != orbit_base::kAllProcessThreadsTid) {
        top_down_view->IncreaseSampleCount(sample_count);
      }

      CallTreeThread* thread_node =
          GetOrCreateThreadNode(top_down_view.get(), tid, process_name, thread_names);
      thread_node->IncreaseSampleCount(sample_count);

      if (resolved_callstack.type() == CallstackInfo::kComplete) {
        AddCallstackToTopDownThread(thread_node, resolved_callstack, sample_count, capture_data);
      } else {
        AddUnwindErrorToTopDownThread(thread_node, resolved_callstack, sample_count, capture_data);
      }
    }
  }
  return top_down_view;
}

[[nodiscard]] static CallTreeNode* AddReversedCallstackToBottomUpViewAndReturnLastFunction(
    CallTreeView* bottom_up_view, const CallstackInfo& resolved_callstack,
    uint64_t callstack_sample_count, const CaptureData& capture_data) {
  CallTreeNode* current_node = bottom_up_view;
  for (uint64_t frame : resolved_callstack.frames()) {
    const std::string& function_name = capture_data.GetFunctionNameByAddress(frame);
    const std::string& module_path = capture_data.GetModulePathByAddress(frame);
    const std::string module_build_id = capture_data.FindModuleBuildIdByAddress(frame).value_or("");
    CallTreeFunction* function_node =
        GetOrCreateFunctionNode(current_node, frame, function_name, module_path, module_build_id);
    function_node->IncreaseSampleCount(callstack_sample_count);
    current_node = function_node;
  }
  return current_node;
}

[[nodiscard]] static CallTreeUnwindErrors* AddUnwindErrorToBottomUpViewAndReturnUnwindingErrorsNode(
    CallTreeView* bottom_up_view, const CallstackInfo& resolved_callstack,
    uint64_t callstack_sample_count, const CaptureData& capture_data) {
  CHECK(!resolved_callstack.frames().empty());
  // Only use the innermost frame for unwind errors.
  uint64_t frame = resolved_callstack.frames(0);
  const std::string& function_name = capture_data.GetFunctionNameByAddress(frame);
  const std::string& module_path = capture_data.GetModulePathByAddress(frame);
  const std::string module_build_id = capture_data.FindModuleBuildIdByAddress(frame).value_or("");
  CallTreeFunction* function_node =
      GetOrCreateFunctionNode(bottom_up_view, frame, function_name, module_path, module_build_id);
  function_node->IncreaseSampleCount(callstack_sample_count);

  CallTreeUnwindErrors* unwind_errors_node = function_node->GetUnwindErrorsOrNull();
  if (unwind_errors_node == nullptr) {
    unwind_errors_node = function_node->AddAndGetUnwindErrors();
  }
  unwind_errors_node->IncreaseSampleCount(callstack_sample_count);
  return unwind_errors_node;
}

std::unique_ptr<CallTreeView> CallTreeView::CreateBottomUpViewFromPostProcessedSamplingData(
    const PostProcessedSamplingData& post_processed_sampling_data,
    const CaptureData& capture_data) {
  auto bottom_up_view = std::make_unique<CallTreeView>();
  const std::string& process_name = capture_data.process_name();
  const absl::flat_hash_map<int32_t, std::string>& thread_names = capture_data.thread_names();

  for (const ThreadSampleData& thread_sample_data :
       post_processed_sampling_data.GetThreadSampleData()) {
    const int32_t tid = thread_sample_data.thread_id;
    if (tid == orbit_base::kAllProcessThreadsTid) {
      continue;
    }

    for (const auto& [callstack_id, sample_count] :
         thread_sample_data.sampled_callstack_id_to_count) {
      const CallstackInfo& resolved_callstack =
          post_processed_sampling_data.GetResolvedCallstack(callstack_id);

      bottom_up_view->IncreaseSampleCount(sample_count);

      CallTreeNode* last_node;
      if (resolved_callstack.type() == CallstackInfo::kComplete) {
        last_node = AddReversedCallstackToBottomUpViewAndReturnLastFunction(
            bottom_up_view.get(), resolved_callstack, sample_count, capture_data);
      } else {
        last_node = AddUnwindErrorToBottomUpViewAndReturnUnwindingErrorsNode(
            bottom_up_view.get(), resolved_callstack, sample_count, capture_data);
      }
      CallTreeThread* thread_node =
          GetOrCreateThreadNode(last_node, tid, process_name, thread_names);
      thread_node->IncreaseSampleCount(sample_count);
    }
  }

  return bottom_up_view;
}
