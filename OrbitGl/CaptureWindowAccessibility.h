// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_WINDOW_ACCESSIBILITY_H_
#define ORBIT_GL_CAPTURE_WINDOW_ACCESSIBILITY_H_

#include <functional>
#include <string>

namespace orbit_gl {
struct A11yRect {
  int left, top, width, height;

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

class GlA11yInterface {
 public:
  virtual int AccessibleChildCount() const = 0;
  virtual GlA11yInterface* AccessibleChild(int) const = 0;
  virtual GlA11yInterface* AccessibleChildAt(int, int) const = 0;
  virtual GlA11yInterface* AccessibleParent() const = 0;

  virtual std::string AccessibleName() const = 0;
  virtual A11yRole AccessibleRole() const = 0;
  virtual A11yRect AccessibleLocalRect() const = 0;
};

}  // namespace orbit_gl

#endif