// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "PickingManager.h"

class PickableMock : public Pickable {
 public:
  void OnPick(int, int) override { picked_ = true; }
  void OnDrag(int, int) override { dragging_ = true; }
  void OnRelease() override {
    dragging_ = false;
    picked_ = false;
  }
  void Draw(GlCanvas*, PickingMode) override {}

  bool Draggable() override { return true; }

  bool picked_ = false;
  bool dragging_ = false;

  void Reset() {
    picked_ = false;
    dragging_ = false;
  }
};

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

// Simulate "rendering" the picking color into a uint32_t target
PickingID MockRenderPickingColor(const Color& col_vec) {
  uint32_t col;
  memcpy(&col, &col_vec[0], sizeof(uint32_t));
  PickingID picking_id = PickingID::Get(col);
  return picking_id;
}

TEST(PickingManager, BasicFunctionality) {
  PickableMock pickable1;
  PickableMock pickable2;
  PickingManager pm;

  Color col_vec1 = pm.GetPickableColor(&pickable1, PickingID::BatcherId::UI);
  Color col_vec2 = pm.GetPickableColor(&pickable2, PickingID::BatcherId::UI);
  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec1).m_Id),
            &pickable1);
  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec2).m_Id),
            &pickable2);
  ASSERT_EQ(pm.GetPickableFromId(0xdeadbeef), nullptr);

  pm.Reset();
  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec1).m_Id),
            nullptr);
  ASSERT_EQ(pm.GetPickableFromId(MockRenderPickingColor(col_vec2).m_Id),
            nullptr);
}

TEST(PickingManager, Callbacks) {
  PickableMock pickable;
  PickingManager pm;

  Color col_vec = pm.GetPickableColor(&pickable, PickingID::BatcherId::UI);
  PickingID id = MockRenderPickingColor(col_vec);
  ASSERT_FALSE(pickable.picked_);
  pm.Pick(id.m_Id, 0, 0);
  ASSERT_TRUE(pickable.picked_);

  pm.Release();
  ASSERT_FALSE(pickable.picked_);

  ASSERT_FALSE(pm.IsDragging());
  pm.Pick(id.m_Id, 0, 0);
  ASSERT_TRUE(pm.IsDragging());
  ASSERT_FALSE(pickable.dragging_);

  pm.Drag(10, 10);
  ASSERT_TRUE(pm.IsDragging());
  ASSERT_TRUE(pickable.dragging_);

  pm.Release();
  ASSERT_FALSE(pm.IsDragging());
  ASSERT_FALSE(pickable.dragging_);
}

TEST(PickingManager, Undraggable) {
  UndraggableMock pickable;
  PickingManager pm;

  Color col_vec = pm.GetPickableColor(&pickable, PickingID::BatcherId::UI);
  PickingID id = MockRenderPickingColor(col_vec);

  ASSERT_FALSE(pm.IsDragging());
  pm.Pick(id.m_Id, 0, 0);
  ASSERT_FALSE(pm.IsDragging());
  ASSERT_FALSE(pickable.dragging_);

  pm.Drag(10, 10);
  ASSERT_FALSE(pm.IsDragging());
  ASSERT_FALSE(pickable.dragging_);
}
