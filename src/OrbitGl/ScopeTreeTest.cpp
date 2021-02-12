// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>

#include "ScopeTree.h"
#include "TextBox.h"

namespace {

TextBox* CreateTextBox(uint64_t start, uint64_t end) {
  static std::vector<std::unique_ptr<TextBox>> text_box_buffer;
  auto text_box = std::make_unique<TextBox>();
  text_box->GetMutableGetTimerInfo().set_start(start);
  text_box->GetMutableGetTimerInfo().set_end(end);
  text_box_buffer.push_back(std::move(text_box));
  return text_box_buffer.back().get();
}

void ValidateTree(const ScopeTree& tree) {
  // The output of tree.Print() is visible by running ctest with --verbose.
  tree.Print();

  // Check that recursively counting nodes produces the same result as Size().
  EXPECT_EQ(tree.Size(), tree.Root()->CountNodesInSubtree());

  // Check that counting nodes from the ScopeTree's depth maps produces the same result as Size().
  EXPECT_EQ(tree.Size(), tree.CountOrderedNodes());

  // Check that the tree does not contain duplicate nodes by counting unique nodes.
  EXPECT_EQ(tree.Size(), tree.Root()->GetAllNodesInSubtree().size());
}

TEST(ScopeTree, TreeCreation) {
  ScopeTree tree;
  EXPECT_EQ(tree.Size(), 1);

  tree.Insert(CreateTextBox(1, 100));
  EXPECT_EQ(tree.Size(), 2);
  tree.Insert(CreateTextBox(1, 9));
  EXPECT_EQ(tree.Size(), 3);
  tree.Insert(CreateTextBox(0, 1));
  tree.Insert(CreateTextBox(2, 4));
  tree.Insert(CreateTextBox(4, 9));
  tree.Insert(CreateTextBox(5, 8));
  tree.Insert(CreateTextBox(0, 200));
  tree.Insert(CreateTextBox(1, 100));
  EXPECT_EQ(tree.Height(), 6);
  EXPECT_EQ(tree.Size(), 9);
  ValidateTree(tree);
}

TEST(ScopeTree, SameTimestamps) {
  ScopeTree tree;
  tree.Insert(CreateTextBox(1, 10));
  tree.Insert(CreateTextBox(1, 10));
  tree.Insert(CreateTextBox(1, 10));
  EXPECT_EQ(tree.Height(), 3);
  EXPECT_EQ(tree.Size(), 4);
  ValidateTree(tree);
}

TEST(ScopeTree, SameStartTimestamps) {
  ScopeTree tree;
  tree.Insert(CreateTextBox(1, 10));
  ValidateTree(tree);
  tree.Insert(CreateTextBox(1, 100));
  ValidateTree(tree);
  tree.Insert(CreateTextBox(1, 50));
  EXPECT_EQ(tree.Height(), 3);
  ValidateTree(tree);
}

TEST(ScopeTree, SameEndTimestamps) {
  ScopeTree tree;
  tree.Insert(CreateTextBox(3, 10));
  tree.Insert(CreateTextBox(1, 10));
  tree.Insert(CreateTextBox(2, 10));
  EXPECT_EQ(tree.Height(), 3);
  EXPECT_EQ(tree.Size(), 4);
  ValidateTree(tree);
}

TEST(ScopeTree, OverlappingTimers) {
  // Overlapping timers should appear at the same depth.
  ScopeTree tree;
  tree.Insert(CreateTextBox(0, 200));  // node 0
  tree.Insert(CreateTextBox(1, 10));   // node 1 fits in node 0
  tree.Insert(CreateTextBox(5, 100));  // node 2 overlaps node 1, fits in node 0
  tree.Insert(CreateTextBox(2, 50));   // node 3 overlaps nodes 1 and 2, fits in node 0
  EXPECT_EQ(tree.Height(), 2);
  EXPECT_EQ(tree.Size(), 5);

  const auto& ordered_nodes_by_depth = tree.GetOrderedNodesByDepth();
  EXPECT_EQ(ordered_nodes_by_depth.at(0).size(), 1);  // root node
  EXPECT_EQ(ordered_nodes_by_depth.at(1).size(), 1);  // node 0
  EXPECT_EQ(ordered_nodes_by_depth.at(2).size(), 3);  // nodes 1, 2 and 3
  ValidateTree(tree);
}

TEST(ScopeTree, EmptyTree) {
  ScopeTree tree;
  ValidateTree(tree);
}

}  // namespace
