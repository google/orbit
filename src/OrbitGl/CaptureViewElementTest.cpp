// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CaptureViewElementTester.h"

namespace orbit_gl {

constexpr float kMarginAfterChild = 10.f;
constexpr float kLeafElementHeight = 20.f;

class CaptureViewElementMock : public CaptureViewElement {
 public:
  explicit CaptureViewElementMock(CaptureViewElement* parent, const Viewport* viewport,
                                  const TimeGraphLayout* layout)
      : CaptureViewElement(parent, viewport, layout) {}

  MOCK_METHOD(EventResult, OnMouseWheel,
              (const Vec2& mouse_pos, int delta, const ModifierKeys& modifiers), (override));
};

class UnitTestCaptureViewLeafElement : public CaptureViewElementMock {
 public:
  explicit UnitTestCaptureViewLeafElement(CaptureViewElement* parent, const Viewport* viewport,
                                          const TimeGraphLayout* layout)
      : CaptureViewElementMock(parent, viewport, layout) {}

  [[nodiscard]] float GetHeight() const override { return kLeafElementHeight; }

 private:
  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override {
    return nullptr;
  }
};

class UnitTestCaptureViewContainerElement : public CaptureViewElementMock {
 public:
  explicit UnitTestCaptureViewContainerElement(CaptureViewElement* parent, const Viewport* viewport,
                                               const TimeGraphLayout* layout,
                                               int children_to_create = 0)
      : CaptureViewElementMock(parent, viewport, layout) {
    for (int i = 0; i < children_to_create; ++i) {
      children_.emplace_back(
          std::make_unique<UnitTestCaptureViewLeafElement>(this, viewport, layout));
    }

    SetWidth(viewport->GetWorldWidth());
    UpdateLayout();
  }

  [[nodiscard]] float GetHeight() const override {
    float result = 0;
    for (CaptureViewElement* child : GetAllChildren()) {
      result += child->GetHeight() + kMarginAfterChild;
    }
    return result;
  }

  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override {
    std::vector<CaptureViewElement*> result;
    for (auto& child : children_) {
      result.push_back(child.get());
    }
    return result;
  };

  void AddChild(std::unique_ptr<CaptureViewElement>&& element) {
    // TODO (b/226376237): This should not be needed
    if (element->GetLayoutFlags() & LayoutFlags::kScaleHorizontallyWithParent) {
      element->SetWidth(GetWidth());
    }
    children_.push_back(std::move(element));
    UpdateLayout();
  }

 protected:
  void DoUpdateLayout() override {
    float current_y = GetPos()[1];

    for (auto& child : GetAllChildren()) {
      child->SetPos(0, current_y);
      current_y += child->GetHeight() + kMarginAfterChild;
    }
  }

 private:
  std::vector<std::unique_ptr<CaptureViewElement>> children_;

  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override {
    return nullptr;
  }
};

TEST(CaptureViewElementTesterTest, PassesAllTestsOnExistingElement) {
  const int kChildCount = 2;
  CaptureViewElementTester tester;
  UnitTestCaptureViewContainerElement container_elem(nullptr, tester.GetViewport(),
                                                     tester.GetLayout(), kChildCount);
  tester.RunTests(&container_elem);
}

const Viewport kViewport(100, 100);
const TimeGraphLayout kLayout;

TEST(CaptureViewElement, IsMouseOverWorksForVisibleElements) {
  UnitTestCaptureViewLeafElement elem(nullptr, &kViewport, &kLayout);
  elem.SetWidth(kViewport.GetWorldWidth());

  EXPECT_FALSE(elem.IsMouseOver(Vec2(-1, -1)));
  EXPECT_TRUE(elem.IsMouseOver(Vec2(kLeafElementHeight / 2, kLeafElementHeight / 2)));
  EXPECT_TRUE(elem.IsMouseOver(elem.GetSize()));
  EXPECT_TRUE(elem.IsMouseOver(Vec2(0, 0)));
}

TEST(CaptureViewElement, IsMouseOverDoesNotWorkForElementsOutsideOfTheParent) {
  const int kChildCount = 2;
  UnitTestCaptureViewContainerElement elem(nullptr, &kViewport, &kLayout, kChildCount);

  CaptureViewElement* child0 = elem.GetAllChildren()[0];

  static_assert(kLeafElementHeight > 5);
  child0->SetPos(0, -5);
  EXPECT_TRUE(child0->IsMouseOver(Vec2(0, 0)));
  EXPECT_FALSE(child0->IsMouseOver(Vec2(0, -1)));
}

TEST(CaptureViewElement, MouseWheelEventRecursesToCorrectChildren) {
  using ::testing::_;
  using ::testing::Exactly;
  using ::testing::Return;

  const int kChildCount = 3;
  UnitTestCaptureViewContainerElement container_elem(nullptr, &kViewport, &kLayout, kChildCount);

  static_assert(kMarginAfterChild > 0);

  const Vec2 kPosOutside(-1, -1);
  const Vec2 kPosBetweenChildren(10, kLeafElementHeight + 1);
  const Vec2 kPosOnChild0(10, kLeafElementHeight);
  const Vec2 kPosOnChild1(10, kLeafElementHeight + kMarginAfterChild);
  const Vec2 kPosOnChild2(10, (kLeafElementHeight + kMarginAfterChild) * 2);

  const int kDelta = 1;

  CaptureViewElementMock* child0 =
      dynamic_cast<CaptureViewElementMock*>(container_elem.GetAllChildren()[0]);
  CaptureViewElementMock* child1 =
      dynamic_cast<CaptureViewElementMock*>(container_elem.GetAllChildren()[1]);
  CaptureViewElementMock* child2 =
      dynamic_cast<CaptureViewElementMock*>(container_elem.GetAllChildren()[2]);

  // Expect the parent to catch all mouse wheel events of children 0 and 1, but not those of child 2
  // since child 2 actually handles the event
  EXPECT_CALL(container_elem, OnMouseWheel(_, kDelta, _))
      .Times(Exactly(3))
      .WillRepeatedly(Return(CaptureViewElement::EventResult::kIgnored));

  EXPECT_CALL(*child0, OnMouseWheel(_, kDelta, _))
      .Times(Exactly(1))
      .WillRepeatedly(Return(CaptureViewElement::EventResult::kIgnored));
  EXPECT_CALL(*child1, OnMouseWheel(_, kDelta, _))
      .Times(Exactly(1))
      .WillRepeatedly(Return(CaptureViewElement::EventResult::kIgnored));
  EXPECT_CALL(*child2, OnMouseWheel(_, kDelta, _))
      .Times(Exactly(1))
      .WillRepeatedly(Return(CaptureViewElement::EventResult::kHandled));

  EXPECT_EQ(CaptureViewElement::EventResult::kIgnored,
            container_elem.HandleMouseWheelEvent(kPosOutside, kDelta));
  EXPECT_EQ(CaptureViewElement::EventResult::kIgnored,
            container_elem.HandleMouseWheelEvent(kPosBetweenChildren, kDelta));
  EXPECT_EQ(CaptureViewElement::EventResult::kIgnored,
            container_elem.HandleMouseWheelEvent(kPosOnChild0, kDelta));
  EXPECT_EQ(CaptureViewElement::EventResult::kIgnored,
            container_elem.HandleMouseWheelEvent(kPosOnChild1, kDelta));
  EXPECT_EQ(CaptureViewElement::EventResult::kHandled,
            container_elem.HandleMouseWheelEvent(kPosOnChild2, kDelta));
}

TEST(CaptureViewElement, RequestUpdateBubblesUpAndIsClearedAfterDrawLoop) {
  CaptureViewElementTester tester;
  UnitTestCaptureViewContainerElement root(nullptr, &kViewport, &kLayout);
  UnitTestCaptureViewContainerElement child(&root, &kViewport, &kLayout, 1);

  // On creating the child, we expect everything to be invalidated as a new child has been added
  tester.SimulateDrawLoopAndCheckFlags(&root, true, true);

  // A "Draw" request should only set the "draw" flag
  child.GetAllChildren()[0]->RequestUpdate(CaptureViewElement::RequestUpdateScope::kDraw);
  tester.SimulateDrawLoopAndCheckFlags(&root, true, false);

  // ... and a request with the default value should also require "UpdatePrimitives"
  child.GetAllChildren()[0]->RequestUpdate();
  tester.SimulateDrawLoopAndCheckFlags(&root, true, true);

  // Finally: There shouldn't be any draws required after a render loop has happened
  tester.SimulateDrawLoopAndCheckFlags(&root, false, false);
}
}  // namespace orbit_gl