// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <tuple>

#include "OrbitGl/Button.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CaptureViewElementTester.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/MockBatcher.h"
#include "OrbitGl/MockTextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"

namespace orbit_gl {

TEST(Button, CaptureViewElementWorksAsIntended) {
  orbit_gl::CaptureViewElementTester tester;
  Button button(nullptr, tester.GetViewport(), tester.GetLayout());

  tester.RunTests(&button);
}

TEST(Button, SizeGettersAndSettersWork) {
  orbit_gl::CaptureViewElementTester tester;
  std::shared_ptr<Button> button =
      std::make_shared<Button>(nullptr, tester.GetViewport(), tester.GetLayout());

  Vec2 size(10.f, 10.f);
  button->SetWidth(size[0]);
  button->SetHeight(size[1]);

  tester.SimulateDrawLoopAndCheckFlags(button.get(), true, true);

  EXPECT_EQ(button->GetWidth(), size[0]);
  EXPECT_EQ(button->GetHeight(), size[1]);
  EXPECT_EQ(button->GetSize(), size);

  // Setting width / height to the same values should not request an update
  button->SetWidth(size[0]);
  button->SetHeight(size[1]);

  tester.SimulateDrawLoopAndCheckFlags(button.get(), false, false);

  // Changing to a different size should affect the property and flags again
  size = Vec2(15.f, 20.f);
  button->SetWidth(size[0]);
  button->SetHeight(size[1]);

  tester.SimulateDrawLoopAndCheckFlags(button.get(), true, true);

  EXPECT_EQ(button->GetWidth(), size[0]);
  EXPECT_EQ(button->GetHeight(), size[1]);
  EXPECT_EQ(button->GetSize(), size);
}

TEST(Button, SizeCannotBeZero) {
  orbit_gl::CaptureViewElementTester tester;
  std::shared_ptr<Button> button =
      std::make_shared<Button>(nullptr, tester.GetViewport(), tester.GetLayout());

  button->SetWidth(0.f);
  button->SetHeight(0.f);

  tester.SimulatePreRender(button.get());

  EXPECT_EQ(button->GetWidth(), tester.GetLayout()->GetMinButtonSize());
  EXPECT_EQ(button->GetHeight(), tester.GetLayout()->GetMinButtonSize());
}

TEST(Button, NameWorksAsExpected) {
  orbit_gl::CaptureViewElementTester tester;
  const std::string name = "UnitTest";
  std::shared_ptr<Button> button =
      std::make_shared<Button>(nullptr, tester.GetViewport(), tester.GetLayout(), name);

  EXPECT_EQ(button->GetName(), name);
}

TEST(Button, MouseReleaseCallback) {
  orbit_gl::CaptureViewElementTester tester;
  Button button(nullptr, tester.GetViewport(), tester.GetLayout());

  uint32_t mouse_released_called = 0;
  auto callback = [&](Button* button_param) {
    if (button_param == &button) mouse_released_called++;
  };
  button.SetMouseReleaseCallback(callback);
  std::ignore =
      button.HandleMouseEvent({CaptureViewElement::MouseEventType::kMouseMove, button.GetPos()});

  uint32_t mouse_released_called_expected = 0;
  button.OnRelease();
  EXPECT_EQ(mouse_released_called, ++mouse_released_called_expected);
  button.OnRelease();
  EXPECT_EQ(mouse_released_called, ++mouse_released_called_expected);
  button.SetMouseReleaseCallback(nullptr);
  button.OnRelease();
  EXPECT_EQ(mouse_released_called, mouse_released_called_expected);

  // Releasing outside of the button shouldn't call the callback.
  EXPECT_EQ(button.HandleMouseEvent({CaptureViewElement::MouseEventType::kMouseLeave}),
            CaptureViewElement::EventResult::kIgnored);
  button.SetMouseReleaseCallback(callback);
  button.OnRelease();
  EXPECT_EQ(mouse_released_called, mouse_released_called_expected);
}

TEST(Button, Rendering) {
  orbit_gl::CaptureViewElementTester tester;
  std::shared_ptr<Button> button =
      std::make_shared<Button>(nullptr, tester.GetViewport(), tester.GetLayout());

  const Vec2 size(400, 50);
  const Vec2 pos(10, 10);
  button->SetWidth(size[0]);
  button->SetHeight(size[1]);
  button->SetPos(pos[0], pos[1]);

  tester.SimulateDrawLoop(button.get(), true, false);

  const MockBatcher& batcher = tester.GetBatcher();
  const MockTextRenderer& text_renderer = tester.GetTextRenderer();

  // I don't really care how the button is represented, but I expect at least a single box to be
  // drawn.
  EXPECT_GE(batcher.GetNumBoxes(), 1);
  EXPECT_TRUE(batcher.IsEverythingInsideRectangle(pos, size));
  EXPECT_TRUE(
      batcher.IsEverythingBetweenZLayers(GlCanvas::kZValueButtonBg, GlCanvas::kZValueButton));

  EXPECT_EQ(text_renderer.GetNumAddTextCalls(), 0);

  tester.SimulateDrawLoop(button.get(), false, true);
  // Verify that `UpdatePrimitives` has no effect - all rendering of the button should be done in
  // `Draw`
  EXPECT_EQ(batcher.GetNumElements(), 0);
  EXPECT_EQ(text_renderer.GetNumAddTextCalls(), 0);
}

}  // namespace orbit_gl