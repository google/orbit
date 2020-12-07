// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_WINDOW_ACCESSIBILITY_H_
#define ORBIT_GL_CAPTURE_WINDOW_ACCESSIBILITY_H_

#include <cstdint>
#include <functional>
#include <string>

namespace orbit_gl {
struct A11yRect {
  int left = 0, top = 0, width = 0, height = 0;

  A11yRect() = default;
  A11yRect(int left, int top, int width, int height)
      : left(left), top(top), width(width), height(height) {}
};

// Copied relevant values from qaccessible.h, extend when needed
enum class A11yRole {
  StaticText = 0x00000029,
  ScrollBar = 0x00000003,
  Chart = 0x00000011,
  Grouping = 0x00000014
};

// Copied relevant values from qaccessible.h, extend when needed
struct A11yState {
  // http://msdn.microsoft.com/en-us/library/ms697270.aspx
  int64_t disabled : 1;  // used to be Unavailable
  int64_t selected : 1;
  int64_t focusable : 1;
  int64_t focused : 1;
  int64_t pressed : 1;
  int64_t checkable : 1;
  int64_t checked : 1;
  int64_t checkStateMixed : 1;  // used to be Mixed
  int64_t readOnly : 1;
  int64_t hotTracked : 1;
  int64_t defaultButton : 1;
  int64_t expanded : 1;
  int64_t collapsed : 1;
  int64_t busy : 1;
  int64_t expandable : 1;
  int64_t marqueed : 1;
  int64_t animated : 1;
  int64_t invisible : 1;
  int64_t offscreen : 1;
  int64_t sizeable : 1;
  int64_t movable : 1;
  int64_t selfVoicing : 1;
  int64_t selectable : 1;
  int64_t linked : 1;
  int64_t traversed : 1;
  int64_t multiSelectable : 1;
  int64_t extSelectable : 1;
  int64_t passwordEdit : 1;  // used to be Protected
  int64_t hasPopup : 1;
  int64_t modal : 1;

  // IA2 - we chose to not add some IA2 states for now
  // Below the ones that seem helpful
  int64_t active : 1;
  int64_t invalid : 1;  // = defunct
  int64_t editable : 1;
  int64_t multiLine : 1;
  int64_t selectableText : 1;
  int64_t supportsAutoCompletion : 1;

  int64_t searchEdit : 1;

  A11yState() { memset(this, 0, sizeof(A11yState)); }
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
  [[nodiscard]] virtual A11yState AccessibleState() const = 0;
};

class GlA11yWidgetBridge : public GlA11yControlInterface {
 public:
  [[nodiscard]] const GlA11yControlInterface* AccessibleParent() const override { return nullptr; }

  [[nodiscard]] std::string AccessibleName() const override { return ""; }
  [[nodiscard]] A11yRole AccessibleRole() const override { return A11yRole::Grouping; }
  [[nodiscard]] A11yRect AccessibleLocalRect() const override { return A11yRect(); }
  [[nodiscard]] A11yState AccessibleState() const { return A11yState(); }
};

}  // namespace orbit_gl

#endif