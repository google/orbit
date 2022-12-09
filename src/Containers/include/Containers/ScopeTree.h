// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTAINERS_SCOPE_TREE_H_
#define CONTAINERS_SCOPE_TREE_H_

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#include "BlockChain.h"
#include "Introspection/Introspection.h"
#include "absl/container/btree_map.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

namespace orbit_containers {

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
  [[nodiscard]] std::string ToString() const {
    std::string result;
    ToString(this, &result);
    return result;
  }

  [[nodiscard]] ScopeNode* GetLastChildBeforeOrAtTime(uint64_t time) const;
  [[nodiscard]] std::vector<ScopeNode*> GetChildrenInRange(uint64_t start, uint64_t end) const;
  [[nodiscard]] const absl::btree_map<uint64_t, ScopeNode*>& GetChildrenByStartTime() const {
    return *children_by_start_time_;
  }

  [[nodiscard]] uint64_t Start() const { return scope_->start(); }
  [[nodiscard]] uint64_t End() const { return scope_->end(); }
  [[nodiscard]] uint32_t Depth() const { return depth_; }
  [[nodiscard]] ScopeNode* Parent() const { return parent_; }
  [[nodiscard]] size_t CountNodesInSubtree() const;
  [[nodiscard]] std::set<const ScopeNode*> GetAllNodesInSubtree() const;
  void SetDepth(uint32_t depth) { depth_ = depth; }
  void SetParent(ScopeNode* parent) { parent_ = parent; }
  ScopeT* GetScope() { return scope_; }

 private:
  [[nodiscard]] ScopeNode* FindDeepestParentForNode(const ScopeNode* node);
  static void ToString(const ScopeNode* node, std::string* str, uint32_t depth = 0);
  static void CountNodesInSubtree(const ScopeNode* node, size_t* count);
  static void GetAllNodesInSubtree(const ScopeNode* node, std::set<const ScopeNode*>* node_set);

 private:
  ScopeT* scope_ = nullptr;
  uint32_t depth_ = 0;
  ScopeNode* parent_ = nullptr;

  // We use std::unique_ptr to work around an issue with absl::btree_map which complains about not
  // knowing the size of ScopeT, which is not needed since ScopeNode only stores a ScopeTree*.
  std::unique_ptr<absl::btree_map<uint64_t, ScopeNode<ScopeT>*>> children_by_start_time_ =
      std::make_unique<absl::btree_map<uint64_t, ScopeNode<ScopeT>*>>();
};

template <typename ScopeT>
class ScopeTree {
 public:
  ScopeTree();
  void Insert(ScopeT* scope);
  void Print() const { ORBIT_LOG("%s", ToString()); }
  [[nodiscard]] std::string ToString() const;

  using ScopeNodeT = ScopeNode<ScopeT>;

  [[nodiscard]] const ScopeNodeT* Root() const { return root_; }
  [[nodiscard]] size_t Size() const { return nodes_.size(); }
  [[nodiscard]] size_t CountOrderedNodesByDepth() const;
  [[nodiscard]] uint32_t Depth() const;
  [[nodiscard]] const absl::btree_map<uint64_t /*start time*/, ScopeNodeT*> GetOrderedNodesAtDepth(
      uint32_t depth) const;
  [[nodiscard]] const ScopeT* FindFirstScopeAtOrAfterTime(uint32_t depth, uint64_t time) const;
  [[nodiscard]] const ScopeT* FindNextScopeAtDepth(const ScopeT& scope) const;
  [[nodiscard]] const ScopeT* FindPreviousScopeAtDepth(const ScopeT& scope) const;
  [[nodiscard]] const ScopeT* FindParent(const ScopeT& scope) const;
  [[nodiscard]] const ScopeT* FindFirstChild(const ScopeT& scope) const;

 private:
  [[nodiscard]] const ScopeNodeT* FindScopeNode(const ScopeT& scope) const;
  [[nodiscard]] ScopeNodeT* CreateNode(ScopeT* scope);
  void UpdateDepthInSubtree(ScopeNodeT* node, uint32_t depth);
  [[nodiscard]] const absl::btree_map<uint32_t /*depth*/,
                                      absl::btree_map<uint64_t /*start time*/, ScopeNodeT*>>&
  GetOrderedNodesByDepth() const {
    return ordered_nodes_by_depth_;
  }

  ScopeNodeT* root_ = nullptr;
  BlockChain<ScopeNodeT, 1024> nodes_;
  absl::btree_map<uint32_t, absl::btree_map<uint64_t, ScopeNodeT*>> ordered_nodes_by_depth_;
};

template <typename ScopeT>
ScopeTree<ScopeT>::ScopeTree() {
  static ScopeT kDefaultScope;
  root_ = CreateNode(&kDefaultScope);
  ordered_nodes_by_depth_[0].emplace(0, root_);
}

template <typename ScopeT>
ScopeNode<ScopeT>* ScopeTree<ScopeT>::CreateNode(ScopeT* scope) {
  ScopeNode<ScopeT>& new_node = nodes_.emplace_back(ScopeNodeT(scope));
  return &new_node;
}

template <typename ScopeT>
const ScopeNode<ScopeT>* ScopeTree<ScopeT>::FindScopeNode(const ScopeT& scope) const {
  for (ScopeNode<ScopeT>* node = root_; node != nullptr;
       node = node->GetLastChildBeforeOrAtTime(scope.start())) {
    if (node->Start() == scope.start() && node->End() == scope.end()) {
      return node;
    }
  }
  return nullptr;
}

template <typename ScopeT>
const ScopeT* ScopeTree<ScopeT>::FindParent(const ScopeT& scope) const {
  const ScopeNode<ScopeT>* node = FindScopeNode(scope);
  ORBIT_CHECK(node != nullptr);
  if (node->Parent() == root_) return nullptr;
  return node->Parent()->GetScope();
}

template <typename ScopeT>
const ScopeT* ScopeTree<ScopeT>::FindFirstChild(const ScopeT& scope) const {
  const ScopeNode<ScopeT>* node = FindScopeNode(scope);
  ORBIT_CHECK(node != nullptr);
  const auto children = node->GetChildrenByStartTime();
  if (children.empty()) return nullptr;
  return children.begin()->second->GetScope();
}
template <typename ScopeT>
const absl::btree_map<uint64_t, ScopeNode<ScopeT>*> ScopeTree<ScopeT>::GetOrderedNodesAtDepth(
    uint32_t depth) const {
  // Scope Tree includes a dummy node at depth 0 and therefore it's 1-indexed.
  depth++;

  if (!GetOrderedNodesByDepth().contains(depth)) {
    return {};
  }
  return GetOrderedNodesByDepth().at(depth);
}

template <typename ScopeT>
const ScopeT* ScopeTree<ScopeT>::FindFirstScopeAtOrAfterTime(uint32_t depth, uint64_t time) const {
  // Scope Tree includes a dummy node at depth 0 and therefore it's 1-indexed.
  depth++;

  if (!ordered_nodes_by_depth_.contains(depth)) return nullptr;
  auto& ordered_nodes = ordered_nodes_by_depth_.at(depth);

  // Find the first node after the provided time.
  auto node_it = ordered_nodes.upper_bound(time);

  // The previous node could also have its ending after the provided time.
  // TODO(http://b/200692451): If we want to use ScopeTree with overlapping timers we are missing
  // some of them.
  auto previous_node_it = node_it;
  if (node_it != ordered_nodes.begin() && (--previous_node_it)->second->GetScope()->end() >= time) {
    node_it = previous_node_it;
  }

  if (node_it == ordered_nodes.end()) {
    return nullptr;
  }
  return node_it->second->GetScope();
}

template <typename ScopeT>
const ScopeT* ScopeTree<ScopeT>::FindNextScopeAtDepth(const ScopeT& scope) const {
  const ScopeNode<ScopeT>* node = FindScopeNode(scope);
  ORBIT_CHECK(node != nullptr);
  auto nodes_at_depth = GetOrderedNodesByDepth().at(node->Depth());
  auto node_it = nodes_at_depth.upper_bound(node->Start());
  if (node_it == nodes_at_depth.end()) return nullptr;
  return node_it->second->GetScope();
}

template <typename ScopeT>
const ScopeT* ScopeTree<ScopeT>::FindPreviousScopeAtDepth(const ScopeT& scope) const {
  const ScopeNode<ScopeT>* node = FindScopeNode(scope);
  ORBIT_CHECK(node != nullptr);
  auto nodes_at_depth = GetOrderedNodesByDepth().at(node->Depth());
  auto node_it = nodes_at_depth.lower_bound(node->Start());
  if (node_it == nodes_at_depth.begin()) return nullptr;
  return (--node_it)->second->GetScope();
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
uint32_t ScopeTree<ScopeT>::Depth() const {
  if (ordered_nodes_by_depth_.empty()) return 0;

  // Since `ordered_nodes_by_depth` is an ordered map, we return the depth of the last level. It
  // shouldn't be empty because we are not erasing nodes.
  auto last_depth_it = --ordered_nodes_by_depth_.end();
  return last_depth_it->first;
}

template <typename ScopeT>
std::string ScopeTree<ScopeT>::ToString() const {
  std::string result =
      absl::StrFormat("ScopeTree %u nodes depth=%u:\n%s\n", Size(), Depth(), root_->ToString());
  return result;
}

template <typename ScopeT>
void ScopeNode<ScopeT>::ToString(const ScopeNode* node, std::string* str, uint32_t depth) {
  absl::StrAppend(
      str, absl::StrFormat("d%u %s ScopeNode(%p) [%lu, %lu]\n", node->Depth(),
                           std::string(depth, ' '), node->scope_, node->Start(), node->End()));
  for (auto& [unused_time, child_node] : node->GetChildrenByStartTime()) {
    ToString(child_node, str, depth + 1);
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
  ORBIT_CHECK(count != nullptr);
  ++(*count);
  for (const auto [unused_time, child] : node->GetChildrenByStartTime()) {
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
  ORBIT_CHECK(node_set != nullptr);
  node_set->insert(node);
  for (const auto [unused_time, child] : node->GetChildrenByStartTime()) {
    GetAllNodesInSubtree(child, node_set);
  }
}

template <typename ScopeT>
ScopeNode<ScopeT>* ScopeNode<ScopeT>::GetLastChildBeforeOrAtTime(uint64_t time) const {
  // Get first child before or exactly at "time".
  if (children_by_start_time_->empty()) return nullptr;
  auto next_node_it = children_by_start_time_->upper_bound(time);
  if (next_node_it == children_by_start_time_->begin()) return nullptr;
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
  if (children_by_start_time_->empty()) return {};
  std::vector<ScopeNode*> nodes;
  for (auto node_it = children_by_start_time_->lower_bound(start);
       node_it != children_by_start_time_->end(); ++node_it) {
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
  node->SetParent(parent_node);

  // Migrate current children of the parent that are encompassed by the new node to the new node.
  for (ScopeNode* encompassed_node : parent_node->GetChildrenInRange(node->Start(), node->End())) {
    parent_node->children_by_start_time_->erase(encompassed_node->Start());
    node->children_by_start_time_->emplace(encompassed_node->Start(), encompassed_node);
    encompassed_node->SetParent(node);
  }

  // Add new node as child of parent_node.
  parent_node->children_by_start_time_->emplace(node->Start(), node);
}

}  // namespace orbit_containers

#endif  // CONTAINERS_SCOPE_TREE_H_
