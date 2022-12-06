// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_VIEW_ELEMENT_TESTER_H_
#define ORBIT_GL_CAPTURE_VIEW_ELEMENT_TESTER_H_

#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/MockBatcher.h"
#include "OrbitGl/MockTextRenderer.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/StaticTimeGraphLayout.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

// Can be used to test basic functionality of classes inheriting from CaptureViewElement to assure
// all required methods are implemented / overriden correctly.
// See CaptureViewElementTest.cpp for usage.
class CaptureViewElementTester {
 public:
  CaptureViewElementTester();

  void RunTests(CaptureViewElement* element);

  Viewport* GetViewport() { return &viewport_; }
  TimeGraphLayout* GetLayout() { return &layout_; }

  // Check if internal flags for `element` are set correctly:
  //  * draw: Expect draw_required_ to be set.
  //  * update_primitives: Expect update_primitives_required_ to be set
  // Use this to verify that `UpdateLayout` or any other layout-changing methods work correctly.
  // You usually want to call this once to check the initial state, then call your setters, then
  // call this again. You will want to check that:
  //  * A call to a setter correctly updates the value, and sets draw_required_ or
  //    update_primitives_required_ as needed.
  //  * A call to a setter that does not change the value will *not* result in any of the flags
  //    to be set.
  // In most cases, you'll want to use `SimulateDrawLoopAndCheckFlags` to combine flag checking
  // and an update / render loop.
  static void CheckDrawFlags(CaptureViewElement* element, bool draw, bool update_primitives);

  // Run layout updates without rendering. This is equal to what would happen during
  // CaptureWindow::PreRender. You'll want to use this when you want to verify that a particular
  // functionality is executed during PreRender, and you don't care about what happens during
  // rendering, or explicitely want to check that rendering is not required.
  // Usually, you should be good with simply using `SimulateDrawLoop`.
  void SimulatePreRender(CaptureViewElement* element);

  // Simulate a cycle of `UpdateLayout`, followed by rendering.
  // Depending on the parameters, `Draw`, `UpdatePrimitives`, or both are executed.
  // Most of the times, you'll probably want to use `SimulateDrawLoopAndCheckFlags` instead.
  void SimulateDrawLoop(CaptureViewElement* element, bool draw, bool update_primitives);

  // Combination of `SimulateDrawLoop` and `CheckDrawFlags` (before and after rendering) for
  // convenience.
  // See `CheckDrawFlags` for a description of flag checking.
  void SimulateDrawLoopAndCheckFlags(CaptureViewElement* element, bool draw,
                                     bool update_primitives);

  const MockBatcher& GetBatcher() const { return batcher_; }
  const MockTextRenderer& GetTextRenderer() const { return text_renderer_; }

 protected:
  Viewport viewport_ = Viewport(100, 100);
  StaticTimeGraphLayout layout_;

 private:
  static void TestWidthPropagationToChildren(CaptureViewElement* element);

  MockBatcher batcher_;
  MockTextRenderer text_renderer_;
  PickingManager picking_manager_;

  PrimitiveAssembler primitive_assembler_;
};

}  // namespace orbit_gl

#endif