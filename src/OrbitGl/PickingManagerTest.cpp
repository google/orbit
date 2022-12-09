// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/PickingManagerTest.h"

#include <gtest/gtest.h>

#include <memory>

class UndraggableMock : public PickableMock {
  bool Draggable() override { return false; }
};

TEST(PickingManager, PickableMock) {
  PickableMock pickable = PickableMock();
  ASSERT_FALSE(pickable.dragging_);
  ASSERT_FALSE(pickable.picked_);
  pickable.OnPick(0, 0);
  ASSERT_TRUE(pickable.picked_);
  pickable.OnDrag(0, 0);
  ASSERT_TRUE(pickable.dragging_);
  pickable.OnRelease();
  ASSERT_FALSE(pickable.dragging_);
  pickable.Reset();
  ASSERT_FALSE(pickable.picked_);
}

TEST(PickingManager, BasicFunctionality) {
  auto pickable1 = std::make_shared<PickableMock>();
  auto pickable2 = std::make_shared<PickableMock>();
  auto pickable3 = std::make_shared<PickableMock>();
  PickingManager pm;

  Color col_vec1 = pm.GetPickableColor(pickable1, BatcherId::kUi);
  Color col_vec2 = pm.GetPickableColor(pickable2, BatcherId::kUi);
  Color col_vec3 = pm.GetPickableColor(pickable3, BatcherId::kUi);
  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec1)), pickable1);
  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec2)), pickable2);

  ASSERT_EQ(pm.GetPickableColor(pickable1, BatcherId::kUi), col_vec1);

  PickingId invalid_id{};
  invalid_id.type = PickingType::kPickable;
  invalid_id.element_id = 0xdead;
  ASSERT_FALSE(pm.GetPickableFromId(invalid_id));

  invalid_id.type = PickingType::kLine;
  ASSERT_DEATH(auto pickable = pm.GetPickableFromId(invalid_id), "PickingType::kPickable");

  pm.Reset();
  ASSERT_FALSE(pm.GetPickableFromId(MockRenderPickingColor(col_vec1)));
  ASSERT_FALSE(pm.GetPickableFromId(MockRenderPickingColor(col_vec2)));
}

TEST(PickingManager, Callbacks) {
  auto pickable = std::make_shared<PickableMock>();
  PickingManager pm;

  Color col_vec = pm.GetPickableColor(pickable, BatcherId::kUi);
  PickingId id = MockRenderPickingColor(col_vec);
  ASSERT_FALSE(pickable->picked_);
  ASSERT_FALSE(pm.IsThisElementPicked(pickable.get()));
  pm.Pick(id, 0, 0);
  ASSERT_TRUE(pickable->picked_);
  ASSERT_TRUE(pm.IsThisElementPicked(pickable.get()));

  pm.Release();
  ASSERT_FALSE(pickable->picked_);
  ASSERT_FALSE(pm.IsThisElementPicked(pickable.get()));

  ASSERT_FALSE(pm.IsDragging());
  pm.Pick(id, 0, 0);
  ASSERT_TRUE(pm.IsDragging());
  ASSERT_FALSE(pickable->dragging_);

  pm.Drag(10, 10);
  ASSERT_TRUE(pm.IsDragging());
  ASSERT_TRUE(pickable->dragging_);

  pm.Release();
  ASSERT_FALSE(pm.IsDragging());
  ASSERT_FALSE(pickable->dragging_);
}

TEST(PickingManager, Undraggable) {
  auto pickable = std::make_shared<UndraggableMock>();
  PickingManager pm;

  Color col_vec = pm.GetPickableColor(pickable, BatcherId::kUi);
  PickingId id = MockRenderPickingColor(col_vec);

  ASSERT_FALSE(pm.IsDragging());
  pm.Pick(id, 0, 0);
  ASSERT_FALSE(pm.IsDragging());
  ASSERT_FALSE(pickable->dragging_);

  pm.Drag(10, 10);
  ASSERT_FALSE(pm.IsDragging());
  ASSERT_FALSE(pickable->dragging_);
}

TEST(PickingManager, RobustnessOnReset) {
  std::shared_ptr<PickableMock> pickable = std::make_shared<PickableMock>();
  PickingManager pm;

  Color col_vec = pm.GetPickableColor(pickable, BatcherId::kUi);
  PickingId id = MockRenderPickingColor(col_vec);
  ASSERT_FALSE(pickable->picked_);
  pm.Pick(id, 0, 0);
  ASSERT_TRUE(pickable->picked_);
  pm.Drag(10, 10);
  ASSERT_TRUE(pickable->dragging_);

  pickable.reset();

  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec)), std::shared_ptr<PickableMock>());
  ASSERT_FALSE(pm.IsDragging());
  pm.Pick(id, 0, 0);
  ASSERT_FALSE(pm.GetPicked());

  pickable = std::make_shared<PickableMock>();
  col_vec = pm.GetPickableColor(pickable, BatcherId::kUi);
  id = MockRenderPickingColor(col_vec);

  pickable.reset(new PickableMock());
}

TEST(PickingManager, Overflow) {
  ASSERT_DEATH((void)PickingId::Create(PickingType::kLine, 1 << PickingId::kElementIDBitSize),
               "kElementIDBitSize");
}