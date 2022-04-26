// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "Button.h"
#include "CaptureViewElementTester.h"

namespace orbit_gl {

TEST(Button, CaptureViewElementWorksAsIntended) {
  orbit_gl::CaptureViewElementTester tester;
  Button button(nullptr, tester.GetViewport(), tester.GetLayout());

  tester.RunTests(&button);
}

TEST(Button, SizeGettersAndSettersWork) {
  orbit_gl::CaptureViewElementTester tester;
  Button button(nullptr, tester.GetViewport(), tester.GetLayout());

  Vec2 size(10.f, 10.f);
  button.SetWidth(size[0]);
  button.SetHeight(size[1]);

  tester.SimulateDrawLoopAndCheckFlags(&button, true, true);

  EXPECT_EQ(button.GetWidth(), size[0]);
  EXPECT_EQ(button.GetHeight(), size[1]);
  EXPECT_EQ(button.GetSize(), size);

  // Setting width / height to the same values should not request an update
  button.SetWidth(size[0]);
  button.SetHeight(size[1]);

  tester.SimulateDrawLoopAndCheckFlags(&button, false, false);

  // Changing to a different size should affect the property and flags again
  size = Vec2(15.f, 20.f);
  button.SetWidth(size[0]);
  button.SetHeight(size[1]);

  tester.SimulateDrawLoopAndCheckFlags(&button, true, true);

  EXPECT_EQ(button.GetWidth(), size[0]);
  EXPECT_EQ(button.GetHeight(), size[1]);
  EXPECT_EQ(button.GetSize(), size);
}

TEST(Button, SizeCannotBeZero) {
  orbit_gl::CaptureViewElementTester tester;
  Button button(nullptr, tester.GetViewport(), tester.GetLayout());

  button.SetWidth(0.f);
  button.SetHeight(0.f);

  tester.SimulatePreRender(&button);

  EXPECT_EQ(button.GetWidth(), tester.GetLayout()->GetMinButtonSize());
  EXPECT_EQ(button.GetHeight(), tester.GetLayout()->GetMinButtonSize());
}

}  // namespace orbit_gl