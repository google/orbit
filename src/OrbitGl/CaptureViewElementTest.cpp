// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "CaptureViewElementTester.h"

namespace orbit_gl {

class UnitTestCaptureViewLeafElement : public CaptureViewElement {
 public:
  explicit UnitTestCaptureViewLeafElement(CaptureViewElement* parent, Viewport* viewport,
                                          TimeGraphLayout* layout)
      : CaptureViewElement(parent, viewport, layout) {}

  [[nodiscard]] float GetHeight() const override { return 10; }

 private:
  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override {
    return nullptr;
  }
};

class UnitTestCaptureViewContainerElement : public CaptureViewElement {
 public:
  explicit UnitTestCaptureViewContainerElement(CaptureViewElement* parent, Viewport* viewport,
                                               TimeGraphLayout* layout, int children_to_create = 0)
      : CaptureViewElement(parent, viewport, layout) {
    for (int i = 0; i < children_to_create; ++i) {
      children_.emplace_back(
          std::make_unique<UnitTestCaptureViewLeafElement>(this, viewport, layout));
    }
  }

  [[nodiscard]] float GetHeight() const override {
    float result = 0;
    for (auto& child : GetAllChildren()) {
      result += child->GetHeight();
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

 private:
  std::vector<std::unique_ptr<UnitTestCaptureViewLeafElement>> children_;

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
}  // namespace orbit_gl