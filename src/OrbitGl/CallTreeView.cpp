// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/CallTreeView.h"

#include <absl/container/flat_hash_map.h>
#include <absl/meta/type_traits.h>
#include <absl/strings/str_format.h>
#include <absl/types/span.h>

#include <memory>
#include <string_view>

#include "ClientData/CallstackInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/ThreadConstants.h"

using orbit_client_data::CallstackInfo;
using orbit_client_data::CallstackType;
using orbit_client_data::CaptureData;
using orbit_client_data::ModuleManager;
using orbit_client_data::PostProcessedSamplingData;
using orbit_client_data::ThreadSampleData;

CallTreeNode::~CallTreeNode() = default;

const std::vector<const CallTreeNode*>& CallTreeNode::children() const {
  if (children_cache_.has_value()) {
    return *children_cache_;
  }

  children_cache_.emplace();
  for (const auto& tid_and_thread : thread_children_) {
    children_cache_->push_back(tid_and_thread.second.get());
  }
  for (const auto& address_and_functions : function_children_) {
    children_cache_->push_back(address_and_functions.second.get());
  }
  if (unwind_errors_child_ != nullptr) {
    children_cache_->push_back(unwind_errors_child_.get());
  }

  for (const auto& error_type_and_unwind_error : unwind_error_type_children_) {
    children_cache_->push_back(error_type_and_unwind_error.second.get());
  }

  return *children_cache_;
}

CallTreeThread* CallTreeNode::GetThreadOrNull(uint32_t thread_id) {
  auto thread_it = thread_children_.find(thread_id);
  if (thread_it == thread_children_.end()) {
    return nullptr;
  }
  return thread_it->second.get();
}

CallTreeThread* CallTreeNode::AddAndGetThread(uint32_t thread_id, std::string thread_name) {
  const auto& [it, inserted] = thread_children_.try_emplace(
      thread_id, std::make_unique<CallTreeThread>(thread_id, std::move(thread_name), this));
  ORBIT_CHECK(inserted);
  children_cache_.reset();
  return it->second.get();
}

CallTreeFunction* CallTreeNode::GetFunctionOrNull(uint64_t function_absolute_address) {
  auto function_it = function_children_.find(function_absolute_address);
  if (function_it == function_children_.end()) {
    return nullptr;
  }
  return function_it->second.get();
}

CallTreeFunction* CallTreeNode::AddAndGetFunction(uint64_t function_absolute_address) {
  const auto& [it, inserted] = function_children_.try_emplace(
      function_absolute_address,
      std::make_unique<CallTreeFunction>(function_absolute_address, this));
  ORBIT_CHECK(inserted);
  children_cache_.reset();
  return it->second.get();
}

CallTreeUnwindErrorType* CallTreeNode::GetUnwindErrorTypeOrNull(CallstackType type) {
  auto unwind_error_it = unwind_error_type_children_.find(type);
  if (unwind_error_it == unwind_error_type_children_.end()) {
    return nullptr;
  }
  return unwind_error_it->second.get();
}

CallTreeUnwindErrorType* CallTreeNode::AddAndGetUnwindErrorType(CallstackType type) {
  const auto& [it, inserted] = unwind_error_type_children_.try_emplace(
      type, std::make_unique<CallTreeUnwindErrorType>(this, type));
  ORBIT_CHECK(inserted);
  children_cache_.reset();
  return it->second.get();
}

CallTreeUnwindErrors* CallTreeNode::GetUnwindErrorsOrNull() { return unwind_errors_child_.get(); }

CallTreeUnwindErrors* CallTreeNode::AddAndGetUnwindErrors() {
  ORBIT_CHECK(unwind_errors_child_ == nullptr);
  unwind_errors_child_ = std::make_unique<CallTreeUnwindErrors>(this);
  children_cache_.reset();
  return unwind_errors_child_.get();
}

std::string CallTreeFunction::RetrieveFunctionName(
    const orbit_client_data::ModuleManager& module_manager,
    const orbit_client_data::CaptureData& capture_data) const {
  const std::string& function_name = orbit_client_data::GetFunctionNameByAddress(
      module_manager, capture_data, function_absolute_address_);
  if (function_name != orbit_client_data::kUnknownFunctionOrModuleName) {
    return function_name;
  }
  return absl::StrFormat("[unknown@%#llx]", function_absolute_address_);
}

std::string CallTreeFunction::RetrieveModulePath(
    const orbit_client_data::ModuleManager& module_manager,
    const orbit_client_data::CaptureData& capture_data) const {
  const auto& [module_path, unused_module_build_id] =
      orbit_client_data::FindModulePathAndBuildIdByAddress(module_manager, capture_data,
                                                           function_absolute_address_);
  return module_path;
}

std::string CallTreeFunction::RetrieveModuleBuildId(
    const orbit_client_data::ModuleManager& module_manager,
    const orbit_client_data::CaptureData& capture_data) const {
  const auto& [unused_module_path, module_build_id] =
      orbit_client_data::FindModulePathAndBuildIdByAddress(module_manager, capture_data,
                                                           function_absolute_address_);
  return module_build_id.value_or("");
}

[[nodiscard]] static CallTreeFunction* GetOrCreateFunctionNode(CallTreeNode* current_node,
                                                               uint64_t frame) {
  CallTreeFunction* function_node = current_node->GetFunctionOrNull(frame);
  if (function_node == nullptr) {
    function_node = current_node->AddAndGetFunction(frame);
  }
  return function_node;
}

[[nodiscard]] static CallTreeUnwindErrorType* GetOrCreateUnwindErrorTypeNode(
    CallTreeNode* current_node, CallstackType error_type) {
  CallTreeUnwindErrorType* unwind_error = current_node->GetUnwindErrorTypeOrNull(error_type);
  if (unwind_error == nullptr) {
    unwind_error = current_node->AddAndGetUnwindErrorType(error_type);
  }
  return unwind_error;
}

static void AddCallstackToTopDownThread(
    CallTreeThread* thread_node, const CallstackInfo& resolved_callstack,
    absl::Span<const orbit_client_data::CallstackEvent> callstack_events) {
  uint64_t callstack_sample_count = callstack_events.size();

  CallTreeNode* current_thread_or_function = thread_node;
  for (auto frame_it = resolved_callstack.frames().rbegin();
       frame_it != resolved_callstack.frames().rend(); ++frame_it) {
    uint64_t frame = *frame_it;

    CallTreeFunction* function_node = GetOrCreateFunctionNode(current_thread_or_function, frame);
    function_node->IncreaseSampleCount(callstack_sample_count);
    current_thread_or_function = function_node;
  }
  current_thread_or_function->AddExclusiveCallstackEvents(callstack_events);
}

static void AddUnwindErrorToTopDownThread(
    CallTreeThread* thread_node, const CallstackInfo& resolved_callstack,
    absl::Span<const orbit_client_data::CallstackEvent> callstack_events) {
  CallTreeUnwindErrors* unwind_errors_node = thread_node->GetUnwindErrorsOrNull();
  if (unwind_errors_node == nullptr) {
    unwind_errors_node = thread_node->AddAndGetUnwindErrors();
  }
  uint64_t callstack_sample_count = callstack_events.size();
  unwind_errors_node->IncreaseSampleCount(callstack_sample_count);

  CallTreeUnwindErrorType* unwind_error_type_node =
      GetOrCreateUnwindErrorTypeNode(unwind_errors_node, resolved_callstack.type());
  unwind_error_type_node->IncreaseSampleCount(callstack_sample_count);

  ORBIT_CHECK(!resolved_callstack.frames().empty());
  // Only use the innermost frame for unwind errors.
  uint64_t frame = resolved_callstack.frames()[0];

  CallTreeFunction* function_node = GetOrCreateFunctionNode(unwind_error_type_node, frame);
  function_node->IncreaseSampleCount(callstack_sample_count);
  function_node->AddExclusiveCallstackEvents(callstack_events);
}

[[nodiscard]] static CallTreeThread* GetOrCreateThreadNode(
    CallTreeNode* current_node, uint32_t tid, std::string_view process_name,
    const absl::flat_hash_map<uint32_t, std::string>& thread_names) {
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
    const ModuleManager* module_manager, const CaptureData* capture_data) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_SCOPED_TIMED_LOG("CreateTopDownViewFromPostProcessedSamplingData");

  auto top_down_view_root = std::make_unique<CallTreeRoot>();
  const std::string& process_name = capture_data->process_name();
  const absl::flat_hash_map<uint32_t, std::string>& thread_names = capture_data->thread_names();

  for (const ThreadSampleData* thread_sample_data :
       post_processed_sampling_data.GetSortedThreadSampleData()) {
    const uint32_t tid = thread_sample_data->thread_id;

    for (const auto& [callstack_id, callstack_events] :
         thread_sample_data->sampled_callstack_id_to_events) {
      uint64_t sample_count = callstack_events.size();

      // Don't count samples from the all-thread case again.
      if (tid != orbit_base::kAllProcessThreadsTid) {
        top_down_view_root->IncreaseSampleCount(sample_count);
      }

      CallTreeThread* thread_node =
          GetOrCreateThreadNode(top_down_view_root.get(), tid, process_name, thread_names);
      thread_node->IncreaseSampleCount(sample_count);

      const CallstackInfo& resolved_callstack =
          post_processed_sampling_data.GetResolvedCallstack(callstack_id);
      if (resolved_callstack.type() == CallstackType::kComplete) {
        AddCallstackToTopDownThread(thread_node, resolved_callstack, callstack_events);
      } else {
        AddUnwindErrorToTopDownThread(thread_node, resolved_callstack, callstack_events);
      }
    }
  }
  return absl::WrapUnique<CallTreeView>(
      new CallTreeView(std::move(top_down_view_root), module_manager, capture_data));
}

[[nodiscard]] static CallTreeNode* AddReversedCallstackToBottomUpViewAndReturnLastFunction(
    CallTreeRoot* bottom_up_view_root, const CallstackInfo& resolved_callstack,
    uint64_t callstack_sample_count) {
  CallTreeNode* current_node = bottom_up_view_root;
  for (uint64_t frame : resolved_callstack.frames()) {
    CallTreeFunction* function_node = GetOrCreateFunctionNode(current_node, frame);
    function_node->IncreaseSampleCount(callstack_sample_count);
    current_node = function_node;
  }
  return current_node;
}

[[nodiscard]] static CallTreeUnwindErrorType*
AddUnwindErrorToBottomUpViewAndReturnUnwindErrorTypeNode(CallTreeRoot* bottom_up_view_root,
                                                         const CallstackInfo& resolved_callstack,
                                                         uint64_t callstack_sample_count) {
  ORBIT_CHECK(!resolved_callstack.frames().empty());
  // Only use the innermost frame for unwind errors.
  uint64_t frame = resolved_callstack.frames()[0];
  CallTreeFunction* function_node = GetOrCreateFunctionNode(bottom_up_view_root, frame);
  function_node->IncreaseSampleCount(callstack_sample_count);

  CallTreeUnwindErrors* unwind_errors_node = function_node->GetUnwindErrorsOrNull();
  if (unwind_errors_node == nullptr) {
    unwind_errors_node = function_node->AddAndGetUnwindErrors();
  }
  unwind_errors_node->IncreaseSampleCount(callstack_sample_count);

  CallTreeUnwindErrorType* unwind_error_type_node =
      GetOrCreateUnwindErrorTypeNode(unwind_errors_node, resolved_callstack.type());
  unwind_error_type_node->IncreaseSampleCount(callstack_sample_count);

  return unwind_error_type_node;
}

std::unique_ptr<CallTreeView> CallTreeView::CreateBottomUpViewFromPostProcessedSamplingData(
    const PostProcessedSamplingData& post_processed_sampling_data,
    const ModuleManager* module_manager, const CaptureData* capture_data) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_SCOPED_TIMED_LOG("CreateBottomUpViewFromPostProcessedSamplingData");

  auto bottom_up_view_root = std::make_unique<CallTreeRoot>();
  const std::string& process_name = capture_data->process_name();
  const absl::flat_hash_map<uint32_t, std::string>& thread_names = capture_data->thread_names();

  for (const ThreadSampleData* thread_sample_data :
       post_processed_sampling_data.GetSortedThreadSampleData()) {
    const uint32_t tid = thread_sample_data->thread_id;
    if (tid == orbit_base::kAllProcessThreadsTid) {
      continue;
    }

    for (const auto& [callstack_id, callstack_events] :
         thread_sample_data->sampled_callstack_id_to_events) {
      uint64_t sample_count = callstack_events.size();
      bottom_up_view_root->IncreaseSampleCount(sample_count);

      const CallstackInfo& resolved_callstack =
          post_processed_sampling_data.GetResolvedCallstack(callstack_id);
      CallTreeNode* last_node{};
      if (resolved_callstack.type() == CallstackType::kComplete) {
        last_node = AddReversedCallstackToBottomUpViewAndReturnLastFunction(
            bottom_up_view_root.get(), resolved_callstack, sample_count);
      } else {
        last_node = AddUnwindErrorToBottomUpViewAndReturnUnwindErrorTypeNode(
            bottom_up_view_root.get(), resolved_callstack, sample_count);
      }
      CallTreeThread* thread_node =
          GetOrCreateThreadNode(last_node, tid, process_name, thread_names);
      thread_node->IncreaseSampleCount(sample_count);
      thread_node->AddExclusiveCallstackEvents(callstack_events);
    }
  }

  return absl::WrapUnique<CallTreeView>(
      new CallTreeView(std::move(bottom_up_view_root), module_manager, capture_data));
}
