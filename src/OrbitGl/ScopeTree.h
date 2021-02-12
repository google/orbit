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
#include "TextBox.h"

// ScopeTree is a layer of abstraction above existing TextBox data. It provides hierachical
// relationship between profiling scopes. The goal is to be able to generate the scope tree with
// different streams of scope data that can arrive out of order. The ScopeTree also maintains an
// ordered map of nodes per depth.

class ScopeNode {
 public:
  ScopeNode() = default;
  ScopeNode(TextBox* text_box, uint64_t id) : text_box_(text_box), id_(id) {}

  void Insert(ScopeNode* node);
  void Print() const { Print(this); }

  [[nodiscard]] ScopeNode* GetLastChildBeforeOrAtTime(uint64_t time) const;
  [[nodiscard]] std::map<uint64_t, ScopeNode*>& GetChildren() { return children_; }
  [[nodiscard]] std::vector<ScopeNode*> GetChildrenInRange(uint64_t start, uint64_t end) const;

  [[nodiscard]] uint64_t Start() const { return text_box_->GetTimerInfo().start(); }
  [[nodiscard]] uint64_t End() const { return text_box_->GetTimerInfo().end(); }
  [[nodiscard]] TextBox* GetTextBox() const { return text_box_; }
  [[nodiscard]] uint32_t Id() const { return id_; }
  [[nodiscard]] uint32_t Height() const;
  [[nodiscard]] uint32_t Depth() const { return depth_; }
  [[nodiscard]] size_t CountNodesInSubtree() const;
  [[nodiscard]] std::set<const ScopeNode*> GetAllNodesInSubtree() const;
  void SetDepth(uint32_t depth) { depth_ = depth; }

 private:
  [[nodiscard]] ScopeNode* FindDeepestParentForNode(const ScopeNode* node);
  static void Print(const ScopeNode* node, uint64_t start_time = 0, uint32_t depth = 0);
  static void FindHeight(const ScopeNode* node, uint32_t* height, uint32_t current_height = 0);
  static void CountNodesInSubtree(const ScopeNode* node, size_t* count);
  static void GetAllNodesInSubtree(const ScopeNode* node, std::set<const ScopeNode*>* node_set);

 private:
  TextBox* text_box_ = nullptr;
  uint64_t id_ = 0;
  uint32_t depth_ = 0;
  std::map<uint64_t, ScopeNode*> children_;
};

class ScopeTree {
 public:
  ScopeTree();
  void Insert(TextBox* timer_info);
  void Print() const;

  [[nodiscard]] const ScopeNode* Root() const { return root_; }
  [[nodiscard]] size_t Size() const { return size_; }
  [[nodiscard]] size_t CountOrderedNodes() const;
  [[nodiscard]] uint32_t Height() const { return root_->Height(); }
  [[nodiscard]] const std::map<uint32_t, std::map<uint64_t, ScopeNode*>>& GetOrderedNodesByDepth()
      const {
    return ordered_nodes_by_depth_;
  }

 private:
  [[nodiscard]] ScopeNode* CreateNode(TextBox* timer_info);
  void UpdateDepthInSubtree(ScopeNode* node, uint32_t depth);

 private:
  size_t size_ = 0;
  ScopeNode* root_ = nullptr;
  BlockChain<ScopeNode, 1024> nodes_;
  std::map<uint32_t, std::map<uint64_t, ScopeNode*>> ordered_nodes_by_depth_;
};

#endif  // ORBIT_GL_SCOPE_TREE_H_
