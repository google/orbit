// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureViewElementTester.h"

#include <gtest/gtest.h>

#include <unordered_map>

orbit_gl::CaptureViewElementTester::CaptureViewElementTester()
    : viewport_(1920, 1080), primitive_assembler_(&batcher_, &picking_manager_) {}

void orbit_gl::CaptureViewElementTester::RunTests(CaptureViewElement* element) {
  TestWidthPropagationToChildren(element);
}

void orbit_gl::CaptureViewElementTester::CheckDrawFlags(CaptureViewElement* element, bool draw,
                                                        bool update_primitives) {
  EXPECT_EQ(element->draw_requested_, draw);
  EXPECT_EQ(element->update_primitives_requested_, update_primitives);
  EXPECT_EQ(element->HasLayoutChanged(), draw || update_primitives);
}

void orbit_gl::CaptureViewElementTester::SimulateDrawLoop(CaptureViewElement* element, bool draw,
                                                          bool update_primitives) {
  primitive_assembler_.StartNewFrame();
  text_renderer_.Clear();

  const int kMaxLayoutLoops = layout_.GetMaxLayoutingLoops();
  int layout_loops = 0;

  do {
    element->UpdateLayout();
  } while (++layout_loops < kMaxLayoutLoops && element->HasLayoutChanged());

  EXPECT_LT(layout_loops, kMaxLayoutLoops);

  if (draw) {
    element->Draw(primitive_assembler_, text_renderer_, CaptureViewElement::DrawContext());
  }
  if (update_primitives) {
    element->UpdatePrimitives(primitive_assembler_, text_renderer_, 0, 0, PickingMode::kNone);
  }
}

void orbit_gl::CaptureViewElementTester::SimulateDrawLoopAndCheckFlags(CaptureViewElement* element,
                                                                       bool draw,
                                                                       bool update_primitives) {
  CheckDrawFlags(element, draw, update_primitives);
  SimulateDrawLoop(element, draw, update_primitives);
  CheckDrawFlags(element, false, false);
}

void orbit_gl::CaptureViewElementTester::TestWidthPropagationToChildren(
    CaptureViewElement* element) {
  const float kWidth = 100, kUpdatedWidth = 50;
  std::unordered_map<CaptureViewElement*, float> old_widths;
  for (auto& child : element->GetAllChildren()) {
    old_widths[child] = child->GetWidth();
  }

  element->SetWidth(kWidth);
  for (auto& child : element->GetAllChildren()) {
    if (child->GetLayoutFlags() & CaptureViewElement::LayoutFlags::kScaleHorizontallyWithParent) {
      EXPECT_EQ(kWidth, child->GetWidth());
    } else {
      EXPECT_EQ(old_widths[child], child->GetWidth());
    }
  }

  element->SetWidth(kUpdatedWidth);
  for (auto& child : element->GetAllChildren()) {
    if (child->GetLayoutFlags() & CaptureViewElement::LayoutFlags::kScaleHorizontallyWithParent) {
      EXPECT_EQ(kUpdatedWidth, child->GetWidth());
    } else {
      EXPECT_EQ(old_widths[child], child->GetWidth());
    }
  }
}
