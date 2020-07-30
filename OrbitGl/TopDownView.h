// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TOP_DOWN_VIEW_H_
#define ORBIT_GL_TOP_DOWN_VIEW_H_

#include "SamplingProfiler.h"
#include "absl/container/flat_hash_map.h"

class TopDownNode {
 public:
  explicit TopDownNode(TopDownNode* parent) : parent_{parent} {}
  virtual ~TopDownNode() = default;

  uint64_t sample_count() const { return sample_count_; }

  void IncreaseSampleCount(uint64_t sample_count_increase) {
    sample_count_ += sample_count_increase;
  }

  // parent(), child_count(), children() are needed by TopDownViewItemModel.
  const TopDownNode* parent() const { return parent_; }

  virtual uint64_t child_count() const = 0;

  virtual std::vector<const TopDownNode*> children() const = 0;

 private:
  TopDownNode* parent_;
  uint64_t sample_count_ = 0;
};

class TopDownFunction;

class TopDownInternalNode : public TopDownNode {
 public:
  explicit TopDownInternalNode(TopDownNode* parent) : TopDownNode{parent} {
    CHECK(parent != nullptr);
  }

  TopDownFunction* GetFunctionOrNull(uint64_t function_absolute_address);

  TopDownFunction* AddAndGetFunction(uint64_t function_absolute_address,
                                     std::string function_name);

  float GetInclusivePercent(uint64_t total_sample_count) const {
    return 100.0f * sample_count() / total_sample_count;
  }

  float GetPercentOfParent() const {
    return 100.0f * sample_count() / parent()->sample_count();
  }

 protected:
  std::map<uint64_t, TopDownFunction> function_nodes_;
};

class TopDownFunction : public TopDownInternalNode {
 public:
  explicit TopDownFunction(uint64_t function_absolute_address,
                           std::string function_name, TopDownNode* parent)
      : TopDownInternalNode{parent},
        function_absolute_address_{function_absolute_address},
        function_name_{std::move(function_name)} {}

  uint64_t function_absolute_address() const {
    return function_absolute_address_;
  }

  const std::string& function_name() const { return function_name_; }

  uint64_t GetExclusiveSampleCount() const {
    uint64_t children_sample_count = 0;
    for (const auto& child_address_and_node : function_nodes_) {
      children_sample_count += child_address_and_node.second.sample_count();
    }
    return sample_count() - children_sample_count;
  }

  float GetExclusivePercent(uint64_t total_sample_count) const {
    return 100.0f * GetExclusiveSampleCount() / total_sample_count;
  }

  uint64_t child_count() const override { return function_nodes_.size(); }

  std::vector<const TopDownNode*> children() const override {
    std::vector<const TopDownNode*> ret;
    for (auto& address_and_node : function_nodes_) {
      ret.push_back(&address_and_node.second);
    }
    return ret;
  }

 private:
  uint64_t function_absolute_address_;
  std::string function_name_;
};

class TopDownThread : public TopDownInternalNode {
 public:
  explicit TopDownThread(int32_t thread_id, std::string thread_name,
                         TopDownNode* parent)
      : TopDownInternalNode{parent},
        thread_id_{thread_id},
        thread_name_{std::move(thread_name)} {}

  int32_t thread_id() const { return thread_id_; }

  const std::string& thread_name() const { return thread_name_; }

  uint64_t child_count() const override { return function_nodes_.size(); }

  std::vector<const TopDownNode*> children() const override {
    std::vector<const TopDownNode*> ret;
    for (auto& address_and_node : function_nodes_) {
      ret.push_back(&address_and_node.second);
    }
    return ret;
  }

 private:
  int32_t thread_id_;
  std::string thread_name_;
};

class TopDownView : public TopDownNode {
 public:
  static std::unique_ptr<TopDownView> CreateFromSamplingProfiler(
      const SamplingProfiler& sampling_profiler,
      const std::string& process_name,
      const std::unordered_map<int32_t, std::string>& thread_names,
      const std::unordered_map<uint64_t, std::string>& function_names);

  TopDownView() : TopDownNode{nullptr} {}

  TopDownThread* GetThreadOrNull(int32_t thread_id);

  TopDownThread* AddAndGetThread(int32_t thread_id, std::string thread_name);

  uint64_t child_count() const override { return thread_nodes_.size(); }

  std::vector<const TopDownNode*> children() const override {
    std::vector<const TopDownNode*> ret;
    for (const auto& tid_and_node : thread_nodes_) {
      ret.push_back(&tid_and_node.second);
    }
    return ret;
  }

 private:
  std::map<int32_t, TopDownThread> thread_nodes_;
};

#endif  // ORBIT_GL_TOP_DOWN_VIEW_H_
