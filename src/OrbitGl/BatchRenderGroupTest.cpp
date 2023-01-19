// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <unordered_map>

#include "OrbitGl/BatchRenderGroup.h"
#include "absl/container/flat_hash_map.h"

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

}  // namespace
}  // namespace orbit_gl