// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_ACCESSIBILITY_ACCESSIBLE_INTERFACE_H_
#define ORBIT_ACCESSIBILITY_ACCESSIBLE_INTERFACE_H_

#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>

namespace orbit_accessibility {

struct AccessibilityRect {
  int left = 0, top = 0, width = 0, height = 0;

  AccessibilityRect() = default;
  AccessibilityRect(int left, int top, int width, int height)
      : left(left), top(top), width(width), height(height) {}
};

/*
 * The constants below are a subset of roles defined in oleacc.h.
 * This is windows-only, but to facilitate compilation on linux and to access those in an
 * easier-to-use enum class, we re-define the required constants here.
 * The constants used by QT are the same and, thus, can be translated directly (see QAccessible.h)
 */
enum class AccessibilityRole {
  NoRole = 0x00000000,      // NOLINT(readability-identifier-naming)
  ScrollBar = 0x00000003,   // NOLINT(readability-identifier-naming)
  Client = 0x0000000A,      // NOLINT(readability-identifier-naming)
  Document = 0x0000000F,    // NOLINT(readability-identifier-naming)
  Pane = 0x00000010,        // NOLINT(readability-identifier-naming)
  Chart = 0x00000011,       // NOLINT(readability-identifier-naming)
  Grouping = 0x00000014,    // NOLINT(readability-identifier-naming)
  PageTab = 0x00000025,     // NOLINT(readability-identifier-naming)
  Graphic = 0x00000028,     // NOLINT(readability-identifier-naming)
  StaticText = 0x00000029,  // NOLINT(readability-identifier-naming)
  Button = 0x0000002B       // NOLINT(readability-identifier-naming)
};

/* Selected state constants as required by QAccessible::State, same reasoning as above.
 * Unlike QAccessible, we're using an Enum instead of a bitfield to be able to define selected
 * states.
 * Curiously, the bit field definition in QAccessible.h does not exactly match the constants
 * defined in oleacc.h - since we're casting this to QAccessible::State later, we stick with the
 * QT-definitions
 */
enum class AccessibilityState : uint64_t {
  Normal = 0,
  Disabled = 1,
  Focusable = 1 << 2,
  Focused = 1 << 3,
  Expanded = 1 << 11,
  Collapsed = 1 << 12,
  Expandable = 1 << 14,
  Offscreen = 1 << 18,
  Movable = 1 << 20
};

inline AccessibilityState operator|(AccessibilityState lhs, AccessibilityState rhs) {
  using T = std::underlying_type_t<AccessibilityState>;
  return static_cast<AccessibilityState>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline AccessibilityState& operator|=(AccessibilityState& lhs, AccessibilityState rhs) {
  lhs = lhs | rhs;
  return lhs;
}

/* Interface for methods required by the Microsoft Automation API. This is a simplified equivalent
 * to QAccessibleInterface. See the documentation in OrbitQt/AccessibilityAdapter.h on how this is
 * used and how it works together with QAccessibleInterface.
 *
 * Used to add accessibility to visible elements inside the OpenGl capture window.
 */
class AccessibleInterface {
 public:
  AccessibleInterface(const AccessibleInterface& rhs) = delete;
  AccessibleInterface(AccessibleInterface&& rhs) = delete;
  AccessibleInterface& operator=(const AccessibleInterface& rhs) = delete;
  AccessibleInterface& operator=(AccessibleInterface&& rhs) = delete;
  AccessibleInterface();
  virtual ~AccessibleInterface();

  [[nodiscard]] virtual int AccessibleChildCount() const = 0;
  [[nodiscard]] virtual const AccessibleInterface* AccessibleChild(int index) const = 0;
  [[nodiscard]] virtual const AccessibleInterface* AccessibleParent() const = 0;

  [[nodiscard]] virtual std::string AccessibleName() const = 0;
  [[nodiscard]] virtual AccessibilityRole AccessibleRole() const = 0;
  [[nodiscard]] virtual AccessibilityRect AccessibleRect() const = 0;
  [[nodiscard]] virtual AccessibilityState AccessibleState() const = 0;
};

}  // namespace orbit_accessibility

#endif