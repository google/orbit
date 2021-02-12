// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SCOPE_TREE_H_
#define ORBIT_GL_SCOPE_TREE_H_

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "BlockChain.h"
#include "OrbitBase/Tracing.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

// ScopeTree is a layer of abstraction above existing scope data. It provides a hierachical
// relationship between profiling scopes. It also maintains an ordered map of nodes per depth. The
// goal is to be able to generate the scope tree with different streams of scope data that can
// arrive out of order. The underlying scope type needs to define the "uint64_t Start()" and
// "uint64_t End()" methods. Note that ScopeTree is not thread safe in its current implementation.

template <typename ScopeT>
class ScopeNode {
 public:
  ScopeNode() = default;
  ScopeNode(ScopeT* scope) : scope_(scope) {}

  void Insert(ScopeNode* node);
  std::string ToString() const {
    std::string result;
    ToString(this, &result);
    return result;
  }

  [[nodiscard]] ScopeNode* GetLastChildBeforeOrAtTime(uint64_t time) const;
  [[nodiscard]] std::vector<ScopeNode*> GetChildrenInRange(uint64_t start, uint64_t end) const;
  [[nodiscard]] std::map<uint64_t, ScopeNode*>& GetChildrenByStartTime() {
    return children_by_start_time_;
  }

  [[nodiscard]] uint64_t Start() const { return scope_->Start(); }
  [[nodiscard]] uint64_t End() const { return scope_->End(); }
  [[nodiscard]] uint32_t Height() const;
  [[nodiscard]] uint32_t Depth() const { return depth_; }
  [[nodiscard]] size_t CountNodesInSubtree() const;
  [[nodiscard]] std::set<const ScopeNode*> GetAllNodesInSubtree() const;
  void SetDepth(uint32_t depth) { depth_ = depth; }

 private:
  [[nodiscard]] ScopeNode* FindDeepestParentForNode(const ScopeNode* node);
  static void ToString(const ScopeNode* node, std::string* str, uint32_t depth = 0);
  static void FindHeight(const ScopeNode* node, uint32_t* height, uint32_t current_height = 0);
  static void CountNodesInSubtree(const ScopeNode* node, size_t* count);
  static void GetAllNodesInSubtree(const ScopeNode* node, std::set<const ScopeNode*>* node_set);

 private:
  ScopeT* scope_ = nullptr;
  uint32_t depth_ = 0;
  std::map<uint64_t, ScopeNode<ScopeT>*> children_by_start_time_;
};

template <typename ScopeT>
class ScopeTree {
 public:
  ScopeTree();
  void Insert(ScopeT* scope);
  void Print() const { LOG("%s", ToString()); }
  std::string ToString() const;

  using ScopeNodeT = ScopeNode<ScopeT>;

  [[nodiscard]] const ScopeNodeT* Root() const { return root_; }
  [[nodiscard]] size_t Size() const { return nodes_.size(); }
  [[nodiscard]] size_t CountOrderedNodesByDepth() const;
  [[nodiscard]] uint32_t Height() const { return root_->Height(); }
  [[nodiscard]] const std::map<uint32_t, std::map<uint64_t /*start time*/, ScopeNodeT*>>&
  GetOrderedNodesByDepth() const {
    return ordered_nodes_by_depth_;
  }

  [[nodiscard]] const ScopeNode* Root() const { return root_; }
  [[nodiscard]] size_t Size() const { return size_; }
  [[nodiscard]] size_t CountOrderedNodes() const;
  [[nodiscard]] uint32_t Height() const { return root_->Height(); }
  [[nodiscard]] const std::map<uint32_t, std::map<uint64_t, ScopeNode*>>& GetOrderedNodesByDepth()
      const {
    return ordered_nodes_by_depth_;
  }

 private:
  [[nodiscard]] ScopeNodeT* CreateNode(ScopeT* scope);
  void UpdateDepthInSubtree(ScopeNodeT* node, uint32_t depth);

 private:
  ScopeNodeT* root_ = nullptr;
  BlockChain<ScopeNodeT, 1024> nodes_;
  std::map<uint32_t, std::map<uint64_t, ScopeNodeT*>> ordered_nodes_by_depth_;
};

template <typename ScopeT>
ScopeTree<ScopeT>::ScopeTree() {
  static ScopeT kDefaultScope;
  root_ = CreateNode(&kDefaultScope);
  ordered_nodes_by_depth_[0].emplace(0, root_);
}

template <typename ScopeT>
ScopeNode<ScopeT>* ScopeTree<ScopeT>::CreateNode(ScopeT* scope) {
  nodes_.push_back(ScopeNodeT(scope));
  ScopeNodeT* node = nodes_.Last();
  return node;
}

template <typename ScopeT>
void ScopeTree<ScopeT>::Insert(ScopeT* scope) {
  ScopeNode<ScopeT>* new_node = CreateNode(scope);
  root_->Insert(new_node);
  // Adjust depths.
  UpdateDepthInSubtree(new_node, new_node->Depth());
}

template <typename ScopeT>
void ScopeTree<ScopeT>::UpdateDepthInSubtree(ScopeNodeT* node, uint32_t new_depth) {
  uint32_t previous_depth = node->Depth();
  uint64_t node_timestamp = node->Start();

  // Remove node from previous depth track.
  if (previous_depth != new_depth) {
    ordered_nodes_by_depth_[previous_depth].erase(node_timestamp);
    node->SetDepth(new_depth);
  }

  // Recurse before inserting the node at new depth to prevent overwriting a child.
  for (auto& [unused_timestamp, child_node] : node->GetChildrenByStartTime()) {
    UpdateDepthInSubtree(child_node, new_depth + 1);
  }

  // Add node to new depth track.
  ordered_nodes_by_depth_[new_depth].insert({node_timestamp, node});
}

template <typename ScopeT>
size_t ScopeTree<ScopeT>::CountOrderedNodesByDepth() const {
  size_t count_from_depth = 0;
  for (auto& [unused_depth, nodes_in_depth] : ordered_nodes_by_depth_) {
    count_from_depth += nodes_in_depth.size();
  }
  return count_from_depth;
}

template <typename ScopeT>
std::string ScopeTree<ScopeT>::ToString() const {
  std::string result =
      absl::StrFormat("ScopeTree %u nodes height=%u:\n%s\n", Size(), Height(), root_->ToString());
  return result;
}

template <typename ScopeT>
void ScopeNode<ScopeT>::ToString(const ScopeNode* node, std::string* str, uint32_t depth) {
  absl::StrAppend(str, absl::StrFormat("d%u %s ScopeNode(%p) [%lu, %lu]\n", node->Depth(),
                                       std::string(depth, ' '), node->scope_, node->scope_->Start(),
                                       node->scope_->End()));
  for (auto& [unused_time, child_node] : node->children_by_start_time_) {
    ToString(child_node, str, depth + 1);
  }
}

template <typename ScopeT>
uint32_t ScopeNode<ScopeT>::Height() const {
  uint32_t max_height = 0;
  FindHeight(this, &max_height);
  return max_height;
}

template <typename ScopeT>
void ScopeNode<ScopeT>::FindHeight(const ScopeNode* node, uint32_t* height,
                                   uint32_t current_height) {
  ORBIT_SCOPE_FUNCTION;
  *height = std::max(*height, current_height);
  for (auto& [unused_time, child_node] : node->children_by_start_time_) {
    FindHeight(child_node, height, current_height + 1);
  }
}

template <typename ScopeT>
size_t ScopeNode<ScopeT>::CountNodesInSubtree() const {
  size_t num_nodes = 0;
  CountNodesInSubtree(this, &num_nodes);
  return num_nodes;
}

template <typename ScopeT>
void ScopeNode<ScopeT>::CountNodesInSubtree(const ScopeNode* node, size_t* count) {
  CHECK(count != nullptr);
  ++(*count);
  for (const auto [unused_time, child] : node->children_by_start_time_) {
    CountNodesInSubtree(child, count);
  }
}

template <typename ScopeT>
std::set<const ScopeNode<ScopeT>*> ScopeNode<ScopeT>::GetAllNodesInSubtree() const {
  std::set<const ScopeNode*> node_set;
  GetAllNodesInSubtree(this, &node_set);
  return node_set;
}

template <typename ScopeT>
void ScopeNode<ScopeT>::GetAllNodesInSubtree(const ScopeNode* node,
                                             std::set<const ScopeNode*>* node_set) {
  CHECK(node_set != nullptr);
  node_set->insert(node);
  for (const auto [unused_time, child] : node->children_by_start_time_) {
    GetAllNodesInSubtree(child, node_set);
  }
}

template <typename ScopeT>
ScopeNode<ScopeT>* ScopeNode<ScopeT>::GetLastChildBeforeOrAtTime(uint64_t time) const {
  // Get first child before or exactly at "time".
  if (children_by_start_time_.empty()) return nullptr;
  auto next_node_it = children_by_start_time_.upper_bound(time);
  if (next_node_it == children_by_start_time_.begin()) return nullptr;
  return (--next_node_it)->second;
}

template <typename ScopeT>
ScopeNode<ScopeT>* ScopeNode<ScopeT>::FindDeepestParentForNode(const ScopeNode* node) {
  // Find the deepest node in our hierarchy that encloses the passed in node's scope.
  ScopeNode* deepest_node = this;
  for (ScopeNode* current_node = this; current_node != nullptr;) {
    current_node = current_node->GetLastChildBeforeOrAtTime(node->Start());
    if (current_node != nullptr && current_node->End() >= node->End()) {
      deepest_node = current_node;
    }
  }
  return deepest_node;
}

template <typename ScopeT>
std::vector<ScopeNode<ScopeT>*> ScopeNode<ScopeT>::GetChildrenInRange(uint64_t start,
                                                                      uint64_t end) const {
  // Get children that are enclosed by start and end inclusively.
  if (children_by_start_time_.empty()) return {};
  std::vector<ScopeNode*> nodes;
  for (auto node_it = children_by_start_time_.lower_bound(start);
       node_it != children_by_start_time_.end(); ++node_it) {
    ScopeNode* node = node_it->second;
    if (node->Start() >= start && node->End() <= end) {
      nodes.push_back(node);
    } else {
      break;
    }
  }
  return nodes;
}

template <typename ScopeT>
void ScopeNode<ScopeT>::Insert(ScopeNode<ScopeT>* node) {
  ORBIT_SCOPE_FUNCTION;

  // Find deepest parent and set depth on node to insert. The depth of descendants will be updated
  // in ScopeTree::UpdateDepthInSubtree as the tree also needs to update another data structure.
  ScopeNode* parent_node = FindDeepestParentForNode(node);
  node->SetDepth(parent_node->Depth() + 1);

  // Migrate current children of the parent that are encompassed by the new node to the new node.
  for (ScopeNode* encompassed_node : parent_node->GetChildrenInRange(node->Start(), node->End())) {
    parent_node->children_by_start_time_.erase(encompassed_node->Start());
    node->children_by_start_time_.emplace(encompassed_node->Start(), encompassed_node);
  }

  // Add new node as child of parent_node.
  parent_node->children_by_start_time_.emplace(node->Start(), node);
}

#endif  // ORBIT_GL_SCOPE_TREE_H_
