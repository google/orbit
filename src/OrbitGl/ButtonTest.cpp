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

TEST(Button, LabelWorksAsExpected) {
  orbit_gl::CaptureViewElementTester tester;
  Button button(nullptr, tester.GetViewport(), tester.GetLayout());

  tester.SimulateDrawLoopAndCheckFlags(&button, true, true);

  const std::string kLabel = "UnitTest";
  button.SetLabel(kLabel);

  tester.SimulateDrawLoopAndCheckFlags(&button, true, false);

  EXPECT_EQ(button.GetLabel(), kLabel);
}

TEST(Button, MouseReleaseCallback) {
  orbit_gl::CaptureViewElementTester tester;
  Button button(nullptr, tester.GetViewport(), tester.GetLayout());

  uint32_t mouse_released_called = 0;
  auto callback = [&](Button* button_param) {
    if (button_param == &button) mouse_released_called++;
  };
  button.SetMouseReleaseCallback(callback);

  uint32_t mouse_released_called_expected = 0;
  button.OnRelease();
  EXPECT_EQ(mouse_released_called, ++mouse_released_called_expected);
  button.OnRelease();
  EXPECT_EQ(mouse_released_called, ++mouse_released_called_expected);
  button.SetMouseReleaseCallback(nullptr);
  button.OnRelease();
  EXPECT_EQ(mouse_released_called, mouse_released_called_expected);

  Button button_with_wrong_callback(nullptr, tester.GetViewport(), tester.GetLayout());
  button_with_wrong_callback.SetMouseReleaseCallback(callback);
  button_with_wrong_callback.OnRelease();
  EXPECT_EQ(mouse_released_called, mouse_released_called_expected);
}

}  // namespace orbit_gl