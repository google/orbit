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
  element->SetWidth(100);
  for (auto& child : element->GetChildren()) {
    EXPECT_EQ(100, child->GetWidth());
  }

  element->SetWidth(50);
  for (auto& child : element->GetChildren()) {
    EXPECT_EQ(50, child->GetWidth());
  }
}
