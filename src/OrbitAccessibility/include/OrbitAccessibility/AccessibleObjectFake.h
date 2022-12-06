// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_ACCESSIBILITY_ACCESSIBLE_OBJECT_FAKE_H_
#define ORBIT_ACCESSIBILITY_ACCESSIBLE_OBJECT_FAKE_H_

#include "OrbitAccessibility/AccessibleInterface.h"

namespace orbit_accessibility {

class AccessibleObjectFake : public AccessibleInterface {
 public:
  explicit AccessibleObjectFake(AccessibleObjectFake* parent) : parent_(parent) {}
  [[nodiscard]] int AccessibleChildCount() const override { return children_.size(); }
  [[nodiscard]] AccessibleInterface* AccessibleChild(int index) const override {
    return children_[index].get();
  }

  [[nodiscard]] AccessibleInterface* AccessibleParent() const override { return parent_; }
  [[nodiscard]] AccessibilityRole AccessibleRole() const override {
    return AccessibilityRole::Grouping;
  }
  [[nodiscard]] AccessibilityState AccessibleState() const override { return AccessibilityState(); }

  [[nodiscard]] AccessibilityRect AccessibleRect() const override {
    AccessibilityRect result;
    if (parent_ == nullptr) {
      return result;
    }

    int parent_idx = -1;
    for (size_t i = 0; i < parent_->children_.size(); ++i) {
      if (parent_->children_[i].get() == this) {
        parent_idx = static_cast<int>(i);
        break;
      }
    }

    result.left = 0;
    result.top = parent_idx;
    result.width = 1000;
    result.height = 1;
    return result;
  }

  [[nodiscard]] std::string AccessibleName() const override { return "Test"; }

  [[nodiscard]] std::vector<std::unique_ptr<AccessibleObjectFake>>& Children() { return children_; }

 private:
  std::vector<std::unique_ptr<AccessibleObjectFake>> children_;
  AccessibleObjectFake* parent_ = nullptr;
};

}  // namespace orbit_accessibility

#endif