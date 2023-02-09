// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CALL_TREE_VIEW_H_
#define ORBIT_GL_CALL_TREE_VIEW_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/node_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/types/span.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "OrbitBase/Logging.h"

class CallTreeThread;
class CallTreeFunction;
class CallTreeUnwindErrors;
class CallTreeUnwindErrorType;

class CallTreeNode {
 public:
  explicit CallTreeNode(CallTreeNode* parent) : parent_{parent} {}
  virtual ~CallTreeNode() = 0;

  // parent(), child_count(), children() are needed by CallTreeViewItemModel.
  [[nodiscard]] const CallTreeNode* parent() const { return parent_; }

  [[nodiscard]] uint64_t child_count() const {
    return thread_children_.size() + function_children_.size() +
           unwind_error_type_children_.size() + (unwind_errors_child_ != nullptr ? 1 : 0);
  }

  [[nodiscard]] uint64_t thread_count() const { return thread_children_.size(); }

  [[nodiscard]] const std::vector<const CallTreeNode*>& children() const;

  [[nodiscard]] CallTreeThread* GetThreadOrNull(uint32_t thread_id);

  [[nodiscard]] CallTreeThread* AddAndGetThread(uint32_t thread_id, std::string thread_name);

  [[nodiscard]] CallTreeFunction* GetFunctionOrNull(uint64_t function_absolute_address);

  [[nodiscard]] CallTreeFunction* AddAndGetFunction(uint64_t function_absolute_address);

  [[nodiscard]] CallTreeUnwindErrorType* GetUnwindErrorTypeOrNull(
      orbit_client_data::CallstackType type);

  [[nodiscard]] CallTreeUnwindErrorType* AddAndGetUnwindErrorType(
      orbit_client_data::CallstackType type);

  [[nodiscard]] CallTreeUnwindErrors* GetUnwindErrorsOrNull();

  [[nodiscard]] CallTreeUnwindErrors* AddAndGetUnwindErrors();

  [[nodiscard]] uint64_t sample_count() const { return sample_count_; }

  void IncreaseSampleCount(uint64_t sample_count_increase) {
    sample_count_ += sample_count_increase;
  }

  [[nodiscard]] float GetInclusivePercent(uint64_t total_sample_count) const {
    return 100.0f * sample_count() / total_sample_count;
  }

  [[nodiscard]] float GetPercentOfParent() const {
    if (parent_ == nullptr) {
      return 100.0f;
    }
    return 100.0f * sample_count() / parent_->sample_count();
  }

  [[nodiscard]] uint64_t GetExclusiveSampleCount() const {
    return exclusive_callstack_events_.size();
  }

  [[nodiscard]] float GetExclusivePercent(uint64_t total_sample_count) const {
    return 100.0f * GetExclusiveSampleCount() / total_sample_count;
  }

  void AddExclusiveCallstackEvents(
      absl::Span<const orbit_client_data::CallstackEvent> callstack_events) {
    exclusive_callstack_events_.insert(exclusive_callstack_events_.end(), callstack_events.begin(),
                                       callstack_events.end());
  }

  [[nodiscard]] const std::vector<orbit_client_data::CallstackEvent>& exclusive_callstack_events()
      const {
    return exclusive_callstack_events_;
  }

 private:
  absl::flat_hash_map<uint32_t, std::unique_ptr<CallTreeThread>> thread_children_{};
  absl::flat_hash_map<uint64_t, std::unique_ptr<CallTreeFunction>> function_children_{};
  absl::flat_hash_map<orbit_client_data::CallstackType, std::unique_ptr<CallTreeUnwindErrorType>>
      unwind_error_type_children_{};
  std::unique_ptr<CallTreeUnwindErrors> unwind_errors_child_;

  CallTreeNode* parent_;
  uint64_t sample_count_ = 0;
  // Note that we are copying the CallstackEvents into the tree.
  std::vector<orbit_client_data::CallstackEvent> exclusive_callstack_events_{};

  // Filled lazily when children() is called, invalidated when children are invalidated.
  mutable std::optional<std::vector<const CallTreeNode*>> children_cache_{};
};

class CallTreeFunction : public CallTreeNode {
 public:
  explicit CallTreeFunction(uint64_t function_absolute_address, CallTreeNode* parent)
      : CallTreeNode{parent}, function_absolute_address_{function_absolute_address} {}
  ~CallTreeFunction() override = default;

  [[nodiscard]] uint64_t function_absolute_address() const { return function_absolute_address_; }

  [[nodiscard]] std::string RetrieveFunctionName(
      const orbit_client_data::ModuleManager& module_manager,
      const orbit_client_data::CaptureData& capture_data) const;

  [[nodiscard]] std::string RetrieveModulePath(
      const orbit_client_data::ModuleManager& module_manager,
      const orbit_client_data::CaptureData& capture_data) const;

  [[nodiscard]] std::string RetrieveModuleBuildId(
      const orbit_client_data::ModuleManager& module_manager,
      const orbit_client_data::CaptureData& capture_data) const;

  [[nodiscard]] std::string RetrieveModuleName(
      const orbit_client_data::ModuleManager& module_manager,
      const orbit_client_data::CaptureData& capture_data) const {
    return std::filesystem::path(RetrieveModulePath(module_manager, capture_data))
        .filename()
        .string();
  }

 private:
  uint64_t function_absolute_address_;
};

class CallTreeThread : public CallTreeNode {
 public:
  explicit CallTreeThread(uint32_t thread_id, std::string thread_name, CallTreeNode* parent)
      : CallTreeNode{parent}, thread_id_{thread_id}, thread_name_{std::move(thread_name)} {}
  ~CallTreeThread() override = default;

  [[nodiscard]] uint32_t thread_id() const { return thread_id_; }

  [[nodiscard]] const std::string& thread_name() const { return thread_name_; }

 private:
  uint32_t thread_id_;
  std::string thread_name_;
};

class CallTreeUnwindErrors : public CallTreeNode {
 public:
  explicit CallTreeUnwindErrors(CallTreeNode* parent) : CallTreeNode{parent} {}
  ~CallTreeUnwindErrors() override = default;
};

class CallTreeUnwindErrorType : public CallTreeNode {
 public:
  explicit CallTreeUnwindErrorType(CallTreeNode* parent,
                                   orbit_client_data::CallstackType error_type)
      : CallTreeNode{parent}, error_type_{error_type} {
    ORBIT_CHECK(error_type != orbit_client_data::CallstackType::kComplete);
  }
  ~CallTreeUnwindErrorType() override = default;

  [[nodiscard]] orbit_client_data::CallstackType error_type() const { return error_type_; }

 private:
  orbit_client_data::CallstackType error_type_;
};

class CallTreeRoot : public CallTreeNode {
 public:
  CallTreeRoot() : CallTreeNode(nullptr) {}
  ~CallTreeRoot() override = default;
};

class CallTreeView {
 public:
  CallTreeView() : call_tree_root_{std::make_unique<CallTreeRoot>()} {}

  [[nodiscard]] static std::unique_ptr<CallTreeView> CreateTopDownViewFromPostProcessedSamplingData(
      const orbit_client_data::PostProcessedSamplingData& post_processed_sampling_data,
      const orbit_client_data::ModuleManager* module_manager,
      const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] static std::unique_ptr<CallTreeView>
  CreateBottomUpViewFromPostProcessedSamplingData(
      const orbit_client_data::PostProcessedSamplingData& post_processed_sampling_data,
      const orbit_client_data::ModuleManager* module_manager,
      const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] const CallTreeRoot* GetCallTreeRoot() const { return call_tree_root_.get(); }

  [[nodiscard]] const orbit_client_data::ModuleManager& GetModuleManager() const {
    ORBIT_CHECK(module_manager_ != nullptr);
    return *module_manager_;
  }

  [[nodiscard]] const orbit_client_data::CaptureData& GetCaptureData() const {
    ORBIT_CHECK(capture_data_ != nullptr);
    return *capture_data_;
  }

  [[nodiscard]] uint64_t sample_count() const { return call_tree_root_->sample_count(); }

 private:
  CallTreeView(std::unique_ptr<CallTreeRoot> call_tree_root,
               const orbit_client_data::ModuleManager* module_manager,
               const orbit_client_data::CaptureData* capture_data)
      : call_tree_root_{std::move(call_tree_root)},
        module_manager_{module_manager},
        capture_data_{capture_data} {
    ORBIT_CHECK(call_tree_root_ != nullptr);
  }

  std::unique_ptr<CallTreeRoot> call_tree_root_;
  const orbit_client_data::ModuleManager* module_manager_{};
  const orbit_client_data::CaptureData* capture_data_{};
};

#endif  // ORBIT_GL_CALL_TREE_VIEW_H_
