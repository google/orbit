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

TEST(PickingManager, PickableMock) {
  PickableMock pickable = PickableMock();
  ASSERT_FALSE(pickable.dragging_);
  ASSERT_FALSE(pickable.picked_);
  pickable.OnPick(0, 0);
  ASSERT_TRUE(pickable.picked_);
}
TEST(PickingManager, BasicFunctionality) { PickingManager pm(); }
