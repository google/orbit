// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <unordered_map>

#include "OrbitGl/BatchRenderGroup.h"
#include "absl/container/flat_hash_map.h"

namespace orbit_gl {
namespace {

TEST(BatchRenderGroupId, IsHashableBasedOnNameAndLayer) {
  BatchRenderGroupManager manager;
  BatchRenderGroupId g1 = manager.CreateId(1, "g1");
  BatchRenderGroupId g2 = manager.CreateId(2, "g2");

  EXPECT_NE(std::hash<BatchRenderGroupId>()(g1), std::hash<BatchRenderGroupId>()(g2));

  g2.layer = 1;
  EXPECT_NE(std::hash<BatchRenderGroupId>()(g1), std::hash<BatchRenderGroupId>()(g2));

  g2.name = "g1";
  EXPECT_EQ(std::hash<BatchRenderGroupId>()(g1), std::hash<BatchRenderGroupId>()(g2));
}

TEST(BatchRenderGroupId, ComparisonOperators) {
  BatchRenderGroupManager manager;
  BatchRenderGroupId g1 = manager.CreateId(1);
  BatchRenderGroupId g2 = manager.CreateId(2);

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

TEST(BatchRenderGroupId, WorksWithHashMap) {
  BatchRenderGroupManager manager;
  BatchRenderGroupId g1 = manager.CreateId(1);
  BatchRenderGroupId g2 = manager.CreateId(2);

  absl::flat_hash_map<BatchRenderGroupId, std::string> hash_map;
  hash_map[g1] = "g1";
  hash_map[g2] = "g2";

  EXPECT_EQ(hash_map.size(), 2);
  EXPECT_EQ(hash_map.count(g1), 1);
  EXPECT_EQ(hash_map.count(g2), 1);

  EXPECT_EQ(hash_map[g1], "g1");
  EXPECT_EQ(hash_map[g2], "g2");
  EXPECT_NE(hash_map[g1], hash_map[g2]);

  BatchRenderGroupId g3 = manager.CreateId(1, "custom");
  BatchRenderGroupId g4 = manager.CreateId(1, "custom");

  ASSERT_NE(g3, g1);
  ASSERT_NE(g3, g2);
  ASSERT_EQ(g3, g4);
  ASSERT_EQ(std::hash<BatchRenderGroupId>()(g3), std::hash<BatchRenderGroupId>()(g3));

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
  BatchRenderGroupManager manager;

  BatchRenderGroupId g1 = manager.CreateId(1, "g1");
  BatchRenderGroupId g2 = manager.CreateId(2, "g2");

  BatchRenderGroupIdComparator comparator = manager.CreateComparator();

  EXPECT_TRUE(comparator(g1, g2));

  // For the same layer and name, they are treated as equal as long as no content has been added
  g2.layer = 1;
  EXPECT_FALSE(comparator(g1, g2));

  // After touching the layers, this should define the new order of rendering
  manager.TouchId(g2);
  manager.TouchId(g1);
  EXPECT_FALSE(comparator(g1, g2));

  // Reseting the order restarts ordering
  manager.ResetOrdering();
  EXPECT_FALSE(comparator(g1, g2));

  manager.TouchId(g1);
  manager.TouchId(g2);
  EXPECT_TRUE(comparator(g1, g2));
}

TEST(BatchRenderGroupManager, SetAndGetState) {
  BatchRenderGroupManager manager;
  BatchRenderGroupId g1 = manager.CreateId();
  BatchRenderGroupState state;

  state.stencil.enabled = !state.stencil.enabled;
  manager.SetGroupState(g1, state);

  EXPECT_EQ(manager.GetGroupState(g1).stencil.enabled, state.stencil.enabled);

  state.stencil.enabled = !state.stencil.enabled;
  manager.SetGroupState(g1, state);

  EXPECT_EQ(manager.GetGroupState(g1).stencil.enabled, state.stencil.enabled);
}

}  // namespace
}  // namespace orbit_gl