// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TESTS_PICKING_MANAGER_TEST_H_
#define ORBIT_GL_TESTS_PICKING_MANAGER_TEST_H_

#include <GteVector.h>
#include <absl/base/casts.h>
#include <stdint.h>

#include <array>

#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PickingManager.h"
#include "absl/base/casts.h"

class PickableMock : public Pickable {
 public:
  void OnPick(int /*x*/, int /*y*/) override { picked_ = true; }
  void OnDrag(int /*x*/, int /*y*/) override { dragging_ = true; }
  void OnRelease() override {
    dragging_ = false;
    picked_ = false;
  }

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
  std::array<uint8_t, 4> color_values{col_vec[0], col_vec[1], col_vec[2], col_vec[3]};
  auto col = absl::bit_cast<uint32_t>(color_values);
  PickingId picking_id = PickingId::FromPixelValue(col);
  return picking_id;
}

#endif
