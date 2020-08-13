// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PickingManagerTest.h"

#include <gtest/gtest.h>

#include <cstring>

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
  PickingManager pm;

  Color col_vec1 = pm.GetPickableColor(pickable1, BatcherId::kUi);
  Color col_vec2 = pm.GetPickableColor(pickable2, BatcherId::kUi);
  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec1)).lock(),
            pickable1);
  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec2)).lock(),
            pickable2);

  PickingId invalid_id;
  invalid_id.type = PickingType::kPickable;
  invalid_id.element_id = 0xdeadbeef;
  ASSERT_TRUE(pm.GetPickableFromId(invalid_id).expired());

  invalid_id.type = PickingType::kLine;
  ASSERT_DEATH(auto pickable = pm.GetPickableFromId(invalid_id),
               "PickingType::kPickable");

  pm.Reset();
  ASSERT_TRUE(pm.GetPickableFromId(MockRenderPickingColor(col_vec1)).expired());
  ASSERT_TRUE(pm.GetPickableFromId(MockRenderPickingColor(col_vec2)).expired());
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

  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec)).lock(),
            std::shared_ptr<PickableMock>());
  ASSERT_FALSE(pm.IsDragging());
  pm.Pick(id, 0, 0);
  ASSERT_TRUE(pm.GetPicked().expired());

  pickable = std::make_shared<PickableMock>();
  col_vec = pm.GetPickableColor(pickable, BatcherId::kUi);
  id = MockRenderPickingColor(col_vec);

  pickable.reset(new PickableMock());
}
