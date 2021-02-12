// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ScopeTree.h"

#include "CoreUtils.h"
#include "OrbitBase/Tracing.h"

ScopeTree::ScopeTree() {
  static TextBox kDefaultTextBox;
  root_ = CreateNode(&kDefaultTextBox);
  ordered_nodes_by_depth_[0].insert({0, root_});
}

ScopeNode* ScopeTree::CreateNode(TextBox* text_box) {
  nodes_.push_back(ScopeNode(text_box, size_++));
  ScopeNode* node = nodes_.Last();
  return node;
}

void ScopeTree::Insert(TextBox* text_box) {
  ScopeNode* new_node = CreateNode(text_box);
  root_->Insert(new_node);
  // Adjust depths.
  UpdateDepthInSubtree(new_node, new_node->Depth());
}

void ScopeTree::UpdateDepthInSubtree(ScopeNode* node, uint32_t new_depth) {
  uint32_t previous_depth = node->Depth();
  uint64_t node_timestamp = node->Start();

  // Remove node from previous depth track.
  if (previous_depth != new_depth) {
    ordered_nodes_by_depth_[previous_depth].erase(node_timestamp);
    node->SetDepth(new_depth);
  }

  // Recurse before inserting the node at new depth to prevent overwriting a child.
  for (auto& [unused_timestamp, child_node] : node->GetChildren()) {
    UpdateDepthInSubtree(child_node, new_depth + 1);
  }

  // Add node to new depth track.
  ordered_nodes_by_depth_[new_depth].insert({node_timestamp, node});
}

size_t ScopeTree::CountOrderedNodes() const {
  size_t count_from_depth = 0;
  for (auto& pair : ordered_nodes_by_depth_) {
    count_from_depth += pair.second.size();
  }
  return count_from_depth;
}

void ScopeTree::Print() const {
  LOG("Printing %u nodes height=%u:", size_, Height());
  root_->Print();
  LOG("");

  for (auto& [depth, ordered_nodes] : ordered_nodes_by_depth_) {
    for (auto& [unused_timestamp, node] : ordered_nodes) {
      LOG("%u: node_depth:%u id:%u", depth, node->Depth(), node->Id());
    }
  }
}

void ScopeNode::Print(const ScopeNode* node, uint64_t start_time, uint32_t depth) {
  LOG("d%u %s%lu ScopeNode(%u) %p [%lu, %lu]", node->Depth(), std::string(depth, ' '), start_time,
      node->id_, node, node->text_box_->GetTimerInfo().start(),
      node->text_box_->GetTimerInfo().end());
  for (auto& pair : node->children_) {
    Print(pair.second, pair.first, depth + 1);
  }
}

void ScopeNode::FindHeight(const ScopeNode* node, uint32_t* height, uint32_t current_height) {
  ORBIT_SCOPE_FUNCTION;
  *height = std::max(*height, current_height);
  for (auto& pair : node->children_) {
    FindHeight(pair.second, height, current_height + 1);
  }
}

size_t ScopeNode::CountNodesInSubtree() const {
  size_t num_nodes = 0;
  CountNodesInSubtree(this, &num_nodes);
  return num_nodes;
}

void ScopeNode::CountNodesInSubtree(const ScopeNode* node, size_t* count) {
  CHECK(count != nullptr);
  ++(*count);
  for (const auto [unused_time, child] : node->children_) {
    CountNodesInSubtree(child, count);
  }
}

std::set<const ScopeNode*> ScopeNode::GetAllNodesInSubtree() const {
  std::set<const ScopeNode*> node_set;
  GetAllNodesInSubtree(this, &node_set);
  return node_set;
}

void ScopeNode::GetAllNodesInSubtree(const ScopeNode* node, std::set<const ScopeNode*>* node_set) {
  CHECK(node_set != nullptr);
  node_set->insert(node);
  for (const auto [unused_time, child] : node->children_) {
    GetAllNodesInSubtree(child, node_set);
  }
}

uint32_t ScopeNode::Height() const {
  uint32_t max_height = 0;
  FindHeight(this, &max_height);
  return max_height;
}

ScopeNode* ScopeNode::GetLastChildBeforeOrAtTime(uint64_t time) const {
  // Get first child before or exactly at "time".
  if (children_.empty()) return nullptr;
  auto next_node_it = children_.upper_bound(time);
  if (next_node_it == children_.begin()) return nullptr;
  return (--next_node_it)->second;
}

ScopeNode* ScopeNode::FindDeepestParentForNode(const ScopeNode* node) {
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

std::vector<ScopeNode*> ScopeNode::GetChildrenInRange(uint64_t start, uint64_t end) const {
  // Get children that are enclosed by start and end inclusively.
  if (children_.empty()) return {};
  auto node_it = children_.lower_bound(start);
  if (node_it == children_.end()) return {};
  std::vector<ScopeNode*> nodes;
  while (node_it != children_.end()) {
    ScopeNode* node = (node_it++)->second;
    if (node->Start() >= start && node->End() <= end) {
      nodes.push_back(node);
    } else {
      break;
    }
  }
  return nodes;
}

void ScopeNode::Insert(ScopeNode* node_to_insert) {
  ORBIT_SCOPE_FUNCTION;

  ScopeNode* parent_node = FindDeepestParentForNode(node_to_insert);

  // Set depth only on the node to insert, the depth of descendants will be updated by the ScopeTree
  // as it also needs to update a secondary data structure. See ScopeTree::UpdateDepthInSubtree.
  node_to_insert->SetDepth(parent_node->Depth() + 1);

  // Find all children encompassed by the node about to be inserted.
  std::vector<ScopeNode*> encompassed_nodes =
      parent_node->GetChildrenInRange(node_to_insert->Start(), node_to_insert->End());

  // Remove all encompassed children from the parent node.
  for (ScopeNode* encompassed_node : encompassed_nodes) {
    parent_node->children_.erase(encompassed_node->Start());
  }

  // Make all encompassed nodes children of the new node.
  for (ScopeNode* encompassed_node : encompassed_nodes) {
    node_to_insert->children_.insert({encompassed_node->Start(), encompassed_node});
  }

  // Add new node as child of parent_node.
  parent_node->children_.insert({node_to_insert->Start(), node_to_insert});
}
