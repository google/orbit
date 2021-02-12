// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <random>
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

uint64_t GetFakeTimeStamp() {
  static uint64_t count = 0;
  return ++count;
}

struct ScopeTimer {
  ScopeTimer(std::vector<TestScope*>* scopes, size_t max_nodes) {
    start = GetFakeTimeStamp();
    max_num_nodes = max_nodes;
    scope_buffer = scopes;
  }
  ~ScopeTimer() {
    if (scope_buffer->size() < max_num_nodes) {
      scope_buffer->push_back(CreateScope(start, GetFakeTimeStamp()));
    }
  }
  uint64_t start;
  size_t max_num_nodes;
  std::vector<TestScope*>* scope_buffer;
};

void CreateNestedTestScopes(size_t max_num_nodes, size_t max_depth, size_t num_siblings_per_depth,
                            std::vector<TestScope*>* scope_buffer, size_t depth = 0) {
  if (depth > max_depth) return;
  if (scope_buffer->size() >= max_num_nodes) return;

  // Use ScopeTimer local variables to generate nested scopes that start at creation time and end at
  // destruction time through the use of RAII. Return scopes through passed in "scope_buffer".
  ScopeTimer timer(scope_buffer, max_num_nodes);
  for (size_t i = 0; i < num_siblings_per_depth; ++i) {
    ScopeTimer inner_timer(scope_buffer, max_num_nodes);
    CreateNestedTestScopes(max_num_nodes, max_depth, num_siblings_per_depth, scope_buffer,
                           depth + 1);
  }
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

TEST(ScopeTree, OutOfOrderScopes) {
  constexpr size_t kMaxNumNodes = 1024;
  constexpr size_t kMaxDepth = 16;
  constexpr size_t kNumSiblingsPerDepth = 4;
  std::vector<TestScope*> test_scopes;
  CreateNestedTestScopes(kMaxNumNodes, kMaxDepth, kNumSiblingsPerDepth, &test_scopes);

  // Create a reference tree from "test_scopes".
  ScopeTree<TestScope> reference_tree;
  for (TestScope* scope : test_scopes) {
    reference_tree.Insert(scope);
  }
  ValidateTree(reference_tree);
  std::string reference_string = reference_tree.ToString();

  // shuffle the elements in "test_scopes" and verify that the resulting trees are the same as the
  // reference tree by comparing their string representation.
  std::random_device rd;
  std::mt19937 gen(rd());
  constexpr int kNumShuffles = 10;
  for (int i = 0; i < kNumShuffles; ++i) {
    std::shuffle(test_scopes.begin(), test_scopes.end(), gen);
    ScopeTree<TestScope> tree;
    for (TestScope* scope : test_scopes) {
      LOG("Adding [%lu, %lu]", scope->start, scope->end);
      tree.Insert(scope);
    }
    ValidateTree(tree);
    std::string tree_string = tree.ToString();
    EXPECT_STREQ(reference_string.c_str(), tree_string.c_str());
  }
}

}  // namespace
