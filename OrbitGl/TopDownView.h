// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TOP_DOWN_VIEW_H_
#define ORBIT_GL_TOP_DOWN_VIEW_H_

#include "CaptureData.h"
#include "Path.h"
#include "absl/container/node_hash_map.h"

class TopDownThread;
class TopDownFunction;

class TopDownNode {
 public:
  explicit TopDownNode(TopDownNode* parent) : parent_{parent} {}
  virtual ~TopDownNode() = default;

  // parent(), child_count(), children() are needed by TopDownViewItemModel.
  [[nodiscard]] const TopDownNode* parent() const { return parent_; }

  [[nodiscard]] uint64_t child_count() const {
    return thread_children_.size() + function_children_.size();
  }

  [[nodiscard]] std::vector<const TopDownNode*> children() const;

  [[nodiscard]] TopDownThread* GetThreadOrNull(int32_t thread_id);

  [[nodiscard]] TopDownThread* AddAndGetThread(int32_t thread_id, std::string thread_name);

  [[nodiscard]] TopDownFunction* GetFunctionOrNull(uint64_t function_absolute_address);

  [[nodiscard]] TopDownFunction* AddAndGetFunction(uint64_t function_absolute_address,
                                                   std::string function_name,
                                                   std::string module_path);

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

  [[nodiscard]] uint64_t GetExclusiveSampleCount() const;

  [[nodiscard]] float GetExclusivePercent(uint64_t total_sample_count) const {
    return 100.0f * GetExclusiveSampleCount() / total_sample_count;
  }

 protected:
  // node_hash_map instead of flat_hash_map as pointer stability is needed for
  // the TopDownNode::parent_ field.
  absl::node_hash_map<int32_t, TopDownThread> thread_children_;
  absl::node_hash_map<uint64_t, TopDownFunction> function_children_;

 private:
  TopDownNode* parent_;
  uint64_t sample_count_ = 0;
};

class TopDownFunction : public TopDownNode {
 public:
  explicit TopDownFunction(uint64_t function_absolute_address, std::string function_name,
                           std::string module_path, TopDownNode* parent)
      : TopDownNode{parent},
        function_absolute_address_{function_absolute_address},
        function_name_{std::move(function_name)},
        module_path_{std::move(module_path)} {}

  [[nodiscard]] uint64_t function_absolute_address() const { return function_absolute_address_; }

  [[nodiscard]] const std::string& function_name() const { return function_name_; }

  [[nodiscard]] const std::string& module_path() const { return module_path_; }

  [[nodiscard]] std::string GetModuleName() const { return Path::GetFileName(module_path()); }

 private:
  uint64_t function_absolute_address_;
  std::string function_name_;
  std::string module_path_;
};

class TopDownThread : public TopDownNode {
 public:
  explicit TopDownThread(int32_t thread_id, std::string thread_name, TopDownNode* parent)
      : TopDownNode{parent}, thread_id_{thread_id}, thread_name_{std::move(thread_name)} {}

  [[nodiscard]] int32_t thread_id() const { return thread_id_; }

  [[nodiscard]] const std::string& thread_name() const { return thread_name_; }

 private:
  int32_t thread_id_;
  std::string thread_name_;
};

class TopDownView : public TopDownNode {
 public:
  [[nodiscard]] static std::unique_ptr<TopDownView> CreateFromSamplingProfiler(
      const SamplingProfiler& sampling_profiler, const CaptureData& capture_data);

  TopDownView() : TopDownNode{nullptr} {}
};

#endif  // ORBIT_GL_TOP_DOWN_VIEW_H_
