// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>

#include "ScopeTree.h"

namespace {

struct TestScope {
  uint64_t Start() const { return start; }
  uint64_t End() const { return end; }
  uint64_t start;
  uint64_t end;
};

TestScope* CreateScope(uint64_t start, uint64_t end) {
  static std::vector<std::unique_ptr<TestScope>> scope_buffer;
  auto scope = std::make_unique<TestScope>();
  scope->start = start;
  scope->end = end;
  scope_buffer.push_back(std::move(scope));
  return scope_buffer.back().get();
}

void ValidateTree(const ScopeTree<TestScope>& tree) {
  // The output of tree.Print() is visible by running ctest with --verbose.
  tree.Print();

  // Check that recursively counting nodes produces the same result as Size().
  EXPECT_EQ(tree.Size(), tree.Root()->CountNodesInSubtree());

  // Check that counting nodes from the ScopeTree's depth maps produces the same result as Size().
  EXPECT_EQ(tree.Size(), tree.CountOrderedNodesByDepth());

  // Check that the tree does not contain duplicate nodes by counting unique nodes.
  EXPECT_EQ(tree.Size(), tree.Root()->GetAllNodesInSubtree().size());
}

TEST(ScopeTree, TreeCreation) {
  ScopeTree<TestScope> tree;
  EXPECT_EQ(tree.Size(), 1);

  tree.Insert(CreateScope(1, 100));
  EXPECT_EQ(tree.Size(), 2);
  tree.Insert(CreateScope(1, 9));
  EXPECT_EQ(tree.Size(), 3);
  tree.Insert(CreateScope(0, 1));
  tree.Insert(CreateScope(2, 4));
  tree.Insert(CreateScope(4, 9));
  tree.Insert(CreateScope(5, 8));
  tree.Insert(CreateScope(0, 200));
  tree.Insert(CreateScope(1, 100));
  EXPECT_EQ(tree.Height(), 6);
  EXPECT_EQ(tree.Size(), 9);
  ValidateTree(tree);
}

TEST(ScopeTree, SameTimestamps) {
  ScopeTree<TestScope> tree;
  tree.Insert(CreateScope(1, 10));
  tree.Insert(CreateScope(1, 10));
  tree.Insert(CreateScope(1, 10));
  EXPECT_EQ(tree.Height(), 3);
  EXPECT_EQ(tree.Size(), 4);
  ValidateTree(tree);
}

TEST(ScopeTree, SameStartTimestamps) {
  ScopeTree<TestScope> tree;
  tree.Insert(CreateScope(1, 10));
  ValidateTree(tree);
  tree.Insert(CreateScope(1, 100));
  ValidateTree(tree);
  tree.Insert(CreateScope(1, 50));
  EXPECT_EQ(tree.Height(), 3);
  ValidateTree(tree);
}

TEST(ScopeTree, SameEndTimestamps) {
  ScopeTree<TestScope> tree;
  tree.Insert(CreateScope(3, 10));
  tree.Insert(CreateScope(1, 10));
  tree.Insert(CreateScope(2, 10));
  EXPECT_EQ(tree.Height(), 3);
  EXPECT_EQ(tree.Size(), 4);
  ValidateTree(tree);
}

TEST(ScopeTree, OverlappingTimers) {
  // Overlapping timers should appear at the same depth.
  ScopeTree<TestScope> tree;
  tree.Insert(CreateScope(0, 200));  // node 0
  tree.Insert(CreateScope(1, 10));   // node 1 fits in node 0
  tree.Insert(CreateScope(5, 100));  // node 2 overlaps node 1, fits in node 0
  tree.Insert(CreateScope(2, 50));   // node 3 overlaps nodes 1 and 2, fits in node 0
  EXPECT_EQ(tree.Height(), 2);
  EXPECT_EQ(tree.Size(), 5);

  const auto& ordered_nodes_by_depth = tree.GetOrderedNodesByDepth();
  EXPECT_EQ(ordered_nodes_by_depth.at(0).size(), 1);  // root node
  EXPECT_EQ(ordered_nodes_by_depth.at(1).size(), 1);  // node 0
  EXPECT_EQ(ordered_nodes_by_depth.at(2).size(), 3);  // nodes 1, 2 and 3
  ValidateTree(tree);
}

TEST(ScopeTree, EmptyTree) {
  ScopeTree<TestScope> tree;
  ValidateTree(tree);
}

}  // namespace
