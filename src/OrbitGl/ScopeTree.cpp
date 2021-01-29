// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ScopeTree.h"

#include "OrbitBase/Tracing.h"

using orbit_client_protos::TimerInfo;

ScopeTree::ScopeTree() {
  static TimerInfo kDefaultTimerInfo;
  root_ = CreateNode(&kDefaultTimerInfo);
}

ScopeNode* ScopeTree::CreateNode(TimerInfo* timer_info) {
  nodes_.push_back(ScopeNode(timer_info, size_++));
  ScopeNode* node = nodes_.Last();
  return node;
}

void ScopeTree::Insert(TimerInfo* timer_info) { root_->Insert(CreateNode(timer_info)); }

void ScopeTree::Print() const {
  LOG("Printing %u nodes height=%u:", size_, Height());
  root_->Print();
  LOG("");
}

void ScopeNode::Print(const ScopeNode* node, uint64_t start_time, uint32_t depth) {
  LOG("%s%lu ScopeNode(%u) %p [%lu, %lu]", std::string(depth, ' '), start_time, node->id_, node,
      node->timer_info_->start(), node->timer_info_->end());
  for (auto& pair : node->children_) {
    Print(pair.second, pair.first, depth + 1);
  }
}

void ScopeNode::FindHeight(const ScopeNode* node, size_t* height, size_t current_height) {
  ORBIT_SCOPE_FUNCTION;
  *height = std::max(*height, current_height);
  for (auto& pair : node->children_) {
    FindHeight(pair.second, height, current_height + 1);
  }
}

size_t ScopeNode::Height() const {
  size_t max_height = 0;
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
  // Get children that are enclosed between start and end inclusively.
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
