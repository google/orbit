// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "CaptureViewElementTester.h"

namespace orbit_gl {

class UnitTestCaptureViewElement : public CaptureViewElement {
 public:
  explicit UnitTestCaptureViewElement(CaptureViewElement* parent, TimeGraph* time_graph,
                                      Viewport* viewport, TimeGraphLayout* layout,
                                      int child_count = 0)
      : CaptureViewElement(parent, time_graph, viewport, layout) {
    for (int i = 0; i < child_count; ++i) {
      children_.emplace_back(
          std::make_unique<UnitTestCaptureViewElement>(this, time_graph, viewport, layout));
    }
  }

  [[nodiscard]] float GetHeight() const override { return 0; }
  [[nodiscard]] virtual std::vector<CaptureViewElement*> GetChildren() const {
    std::vector<CaptureViewElement*> result;
    for (auto& child : children_) {
      result.push_back(child.get());
    }
    return result;
  };

 private:
  std::vector<std::unique_ptr<UnitTestCaptureViewElement>> children_;

  [[nodiscard]] std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() override {
    return nullptr;
  }
};

TEST(CaptureViewElementTesterTest, PassesAllTestsOnExistingElement) {
  CaptureViewElementTester tester;
  UnitTestCaptureViewElement container_elem(nullptr, nullptr, tester.GetViewport(),
                                            tester.GetLayout(), 2);
  tester.RunTests(&container_elem);
}
}  // namespace orbit_gl