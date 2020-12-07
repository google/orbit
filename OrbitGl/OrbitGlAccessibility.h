// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_WINDOW_ACCESSIBILITY_H_
#define ORBIT_GL_CAPTURE_WINDOW_ACCESSIBILITY_H_

#include <functional>
#include <string>

namespace orbit_gl {
struct A11yRect {
  int left = 0, top = 0, width = 0, height = 0;

  A11yRect() = default;
  A11yRect(int left, int top, int width, int height)
      : left(left), top(top), width(width), height(height) {}

  void offset_by(int left, int top) {
    this->left += left;
    this->top += top;
  }
};

// Copied relevant values from qaccessible.h, extend when needed
enum class A11yRole {
  StaticText = 0x00000029,
  ScrollBar = 0x00000003,
  Chart = 0x00000011,
  Grouping = 0x00000014
};

class GlA11yControlInterface {
 public:
  [[nodiscard]] virtual int AccessibleChildCount() const = 0;
  [[nodiscard]] virtual const GlA11yControlInterface* AccessibleChild(int index) const = 0;
  [[nodiscard]] virtual const GlA11yControlInterface* AccessibleChildAt(int x, int y) const = 0;
  [[nodiscard]] virtual const GlA11yControlInterface* AccessibleParent() const = 0;

  [[nodiscard]] virtual std::string AccessibleName() const = 0;
  [[nodiscard]] virtual A11yRole AccessibleRole() const = 0;
  [[nodiscard]] virtual A11yRect AccessibleLocalRect() const = 0;
};

class GlA11yWidgetBridge : public GlA11yControlInterface {
 public:
  [[nodiscard]] const GlA11yControlInterface* AccessibleParent() const override { return nullptr; }

  [[nodiscard]] std::string AccessibleName() const override { return ""; }
  [[nodiscard]] A11yRole AccessibleRole() const override { return A11yRole::Grouping; }
  [[nodiscard]] A11yRect AccessibleLocalRect() const override { return A11yRect(); }
};

}  // namespace orbit_gl

#endif