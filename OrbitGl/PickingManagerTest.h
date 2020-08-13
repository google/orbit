// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TESTS_PICKING_MANAGER_TEST_H_
#define ORBIT_GL_TESTS_PICKING_MANAGER_TEST_H_

#include "GlCanvas.h"
#include "PickingManager.h"
#include "absl/base/casts.h"

class PickableMock : public Pickable {
 public:
  void OnPick(int /*x*/, int /*y*/) override { picked_ = true; }
  void OnDrag(int /*x*/, int /*y*/) override { dragging_ = true; }
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

// Simulate "rendering" the picking color into a uint32_t target
inline PickingId MockRenderPickingColor(const Color& col_vec) {
  uint32_t col = absl::bit_cast<uint32_t, Color>(col_vec);
  PickingId picking_id = PickingId::FromPixelValue(col);
  return picking_id;
}

#endif
