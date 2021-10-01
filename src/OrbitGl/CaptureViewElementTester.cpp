// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureViewElementTester.h"

#include <gtest/gtest.h>

void orbit_gl::CaptureViewElementTester::RunTests(CaptureViewElement* element) {
  TestWidthPropagationToChildren(element);
}

void orbit_gl::CaptureViewElementTester::TestWidthPropagationToChildren(
    CaptureViewElement* element) {
  const float kWidth = 100, kUpdatedWidth = 50;

  element->SetWidth(kWidth);
  for (auto& child : element->GetChildren()) {
    if (child->GetLayoutFlags() & CaptureViewElement::LayoutFlags::kScaleHorizontallyWithParent) {
      EXPECT_EQ(kWidth, child->GetWidth());
    }
  }

  element->SetWidth(kUpdatedWidth);
  for (auto& child : element->GetChildren()) {
    if (child->GetLayoutFlags() & CaptureViewElement::LayoutFlags::kScaleHorizontallyWithParent) {
      EXPECT_EQ(kUpdatedWidth, child->GetWidth());
    }
  }
}
