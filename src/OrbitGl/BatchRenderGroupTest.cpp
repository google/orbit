// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <gtest/gtest.h>

#include <string>

#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

namespace {

TEST(BatchRenderGroupId, ComparisonOperators) {
  BatchRenderGroupId g1(1);
  BatchRenderGroupId g2(2);

  EXPECT_FALSE(g1 == g2);
  EXPECT_TRUE(g1 != g2);

  g2.layer = 1;

  EXPECT_TRUE(g1 == g2);
  EXPECT_FALSE(g1 != g2);

  g1.name = "g1";
  g2.name = "g2";

  EXPECT_FALSE(g1 == g2);
  EXPECT_TRUE(g1 != g2);
}

TEST(BatchRenderGroupId, WorksWithHashMapBasedOnNameAndLayer) {
  BatchRenderGroupId g1(1);
  BatchRenderGroupId g2(2);

  absl::flat_hash_map<BatchRenderGroupId, std::string> hash_map;
  hash_map[g1] = "g1";
  hash_map[g2] = "g2";

  EXPECT_EQ(hash_map.size(), 2);
  EXPECT_EQ(hash_map.count(g1), 1);
  EXPECT_EQ(hash_map.count(g2), 1);

  EXPECT_EQ(hash_map[g1], "g1");
  EXPECT_EQ(hash_map[g2], "g2");
  EXPECT_NE(hash_map[g1], hash_map[g2]);

  BatchRenderGroupId g3(1, "custom");
  BatchRenderGroupId g4(1, "custom");

  ASSERT_NE(g3, g1);
  ASSERT_NE(g3, g2);
  ASSERT_EQ(g3, g4);

  hash_map[g3] = "custom";
  EXPECT_EQ(hash_map.size(), 3);
  hash_map[g4] = "custom";

  EXPECT_EQ(hash_map.size(), 3);
  EXPECT_EQ(hash_map.count(g1), 1);
  EXPECT_EQ(hash_map.count(g2), 1);
  EXPECT_EQ(hash_map.count(g3), 1);
  EXPECT_EQ(hash_map.count(g4), 1);

  EXPECT_EQ(hash_map[g1], "g1");
  EXPECT_EQ(hash_map[g2], "g2");
  EXPECT_EQ(hash_map[g3], "custom");
  EXPECT_EQ(hash_map[g4], "custom");
}

TEST(BatchRenderGroupId, OrderingComparator) {
  // The naming scheme "parent|child" is not enforced by the group ID. It will be upheld by the
  // CaptureViewElement implementation though.
  BatchRenderGroupId parent_group(2, "cve_001");
  BatchRenderGroupId child_group(1, "cve_001|cve_002");

  EXPECT_TRUE(parent_group > child_group);
  EXPECT_TRUE(parent_group >= child_group);
  EXPECT_FALSE(parent_group < child_group);
  EXPECT_FALSE(parent_group <= child_group);

  // For the same layer, groups will be sorted by their name - with the convention above, this
  // assure parents are always rendered before their children.
  parent_group.layer = 1;
  EXPECT_TRUE(parent_group < child_group);
  EXPECT_TRUE(parent_group <= child_group);
  EXPECT_FALSE(parent_group > child_group);
  EXPECT_FALSE(parent_group >= child_group);

  child_group.name = "cve_000";
  // After changing the name, the order should be affected
  EXPECT_TRUE(parent_group > child_group);
  EXPECT_TRUE(parent_group >= child_group);
  EXPECT_FALSE(parent_group < child_group);
  EXPECT_FALSE(parent_group <= child_group);
}

TEST(BatchRenderGroupManager, SetAndGetState) {
  BatchRenderGroupStateManager manager;
  BatchRenderGroupId g1;
  BatchRenderGroupState state;

  state.stencil.enabled = !state.stencil.enabled;
  manager.SetGroupState(g1.name, state);

  EXPECT_EQ(manager.GetGroupState(g1.name).stencil.enabled, state.stencil.enabled);

  state.stencil.enabled = !state.stencil.enabled;
  manager.SetGroupState(g1.name, state);

  EXPECT_EQ(manager.GetGroupState(g1.name).stencil.enabled, state.stencil.enabled);
}

TEST(StencilConfig, ClipAtParent) {
  StencilConfig parent;
  StencilConfig child;

  parent.enabled = true;
  child.enabled = true;

  parent.pos = {10, 20};
  child.pos = {20, 30};

  parent.size = {100, 50};
  child.size = {10, 10};

  // Fully contained child is unchanged
  EXPECT_EQ(ClipStencil(child, parent), child);

  // Child bottom-right outside of the parent
  child.size = {120, 120};

  StencilConfig expectation;
  expectation.enabled = true;
  expectation.pos = {20, 30};
  expectation.size = {90, 40};
  EXPECT_EQ(ClipStencil(child, parent), expectation);

  // Child top-left outside of the parent
  child.pos = {0, 0};
  child.size = {30, 30};
  expectation.pos = parent.pos;
  expectation.size = {20, 10};

  // Child is larger than the parent
  child.pos = {0, 0};
  child.size = {120, 120};
  expectation = parent;
  EXPECT_EQ(ClipStencil(child, parent), expectation);

  // Disabled child inherits all values from its parent
  child.enabled = false;
  EXPECT_EQ(ClipStencil(child, parent), parent);

  // Disabled parent has no effect
  child.size = {120, 120};
  child.pos = {0, 0};
  parent.enabled = false;
  child.enabled = true;
  expectation = child;
  EXPECT_EQ(ClipStencil(child, parent), expectation);
}

}  // namespace
}  // namespace orbit_gl