// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TESTS_ACCESSIBILITY_INTERFACE_MOCK_H_
#define ORBIT_GL_TESTS_ACCESSIBILITY_INTERFACE_MOCK_H_

#include "OrbitGlAccessibility.h"

namespace orbit_gl_tests {

class TestA11yImpl : public orbit_gl::GlAccessibleInterface {
 public:
  TestA11yImpl(TestA11yImpl* parent) : GlAccessibleInterface(), parent_(parent) {}
  [[nodiscard]] int AccessibleChildCount() const override { return children_.size(); }
  [[nodiscard]] GlAccessibleInterface* AccessibleChild(int index) const override {
    return children_[index].get();
  }

  [[nodiscard]] GlAccessibleInterface* AccessibleParent() const { return parent_; }
  [[nodiscard]] orbit_gl::A11yRole AccessibleRole() const { return orbit_gl::A11yRole::Grouping; }
  [[nodiscard]] orbit_gl::A11yState AccessibleState() const { return orbit_gl::A11yState(); }

  [[nodiscard]] orbit_gl::A11yRect AccessibleLocalRect() const {
    orbit_gl::A11yRect result;
    if (parent_ == nullptr) {
      return result;
    }

    int parent_idx = -1;
    for (int i = 0; i < parent_->children_.size(); ++i) {
      if (parent_->children_[i].get() == this) {
        parent_idx = i;
        break;
      }
    }

    result.left = 0;
    result.top = parent_idx;
    result.width = 1000;
    result.height = 1;
    return result;
  }

  [[nodiscard]] std::string AccessibleName() const { return "Test"; }

  [[nodiscard]] std::vector<std::unique_ptr<TestA11yImpl>>& Children() { return children_; }

 private:
  std::vector<std::unique_ptr<TestA11yImpl>> children_;
  TestA11yImpl* parent_ = nullptr;
};

}  // namespace orbit_gl_tests

#endif