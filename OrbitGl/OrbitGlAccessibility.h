// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ORBIT_GL_ACCESSIBILITY_H_
#define ORBIT_GL_ORBIT_GL_ACCESSIBILITY_H_

#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <functional>
#include <string>

namespace orbit_gl {
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
  NoRole = 0x00000000,
  ScrollBar = 0x00000003,
  Client = 0x0000000A,
  Document = 0x0000000F,
  Pane = 0x00000010,
  Chart = 0x00000011,
  Grouping = 0x00000014,
  PageTab = 0x00000025,
  Graphic = 0x00000028,
  StaticText = 0x00000029
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
  Selected = 1 << 1,
  Focusable = 1 << 2,
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

class GlAccessibleInterface;

/* Singleton class, keeps track of created and destroyed GlAccessibleInterface instances.
 * This is required to respond to the destruction of interfaces outside of OrbitGl.
 *
 * orbit_qt::AccessibilityAdapter will register itself for the "SetOnUnregisterCallback" callback.
 * See OrbitQt/AccessibilityAdapter.h for more documentation.
 */
class GlAccessibleInterfaceRegistry {
 public:
  using Callback = std::function<void(GlAccessibleInterface*)>;

  void Register(GlAccessibleInterface* iface);
  void Unregister(GlAccessibleInterface* iface);

  void SetOnRegisterCallback(Callback callback);
  void SetOnUnregisterCallback(Callback callback);

  [[nodiscard]] bool Exists(GlAccessibleInterface* iface) { return interfaces_.contains(iface); }

  static GlAccessibleInterfaceRegistry& Get();

 private:
  GlAccessibleInterfaceRegistry(){};
  ~GlAccessibleInterfaceRegistry();

  absl::flat_hash_set<GlAccessibleInterface*> interfaces_;
  Callback on_registered_ = nullptr;
  Callback on_unregistered_ = nullptr;
};

/* Interface for methods required by the Microsoft Automation API. This is a simplified equivalent
 * to QAccessibleInterface. See the documentation in OrbitQt/AccessibilityAdapter.h on how this is
 * used and how it works together with QAccessibleInterface.
 *
 * Used to add accessibility to visible elements inside the OpenGl capture window.
 */
class GlAccessibleInterface {
 public:
  GlAccessibleInterface(const GlAccessibleInterface& rhs) = delete;
  GlAccessibleInterface(GlAccessibleInterface&& rhs) = delete;
  GlAccessibleInterface& operator=(const GlAccessibleInterface& rhs) = delete;
  GlAccessibleInterface& operator=(GlAccessibleInterface&& rhs) = delete;
  GlAccessibleInterface() { GlAccessibleInterfaceRegistry::Get().Register(this); }
  virtual ~GlAccessibleInterface() { GlAccessibleInterfaceRegistry::Get().Unregister(this); }

  [[nodiscard]] virtual int AccessibleChildCount() const = 0;
  [[nodiscard]] virtual const GlAccessibleInterface* AccessibleChild(int index) const = 0;
  [[nodiscard]] virtual const GlAccessibleInterface* AccessibleParent() const = 0;

  [[nodiscard]] virtual std::string AccessibleName() const = 0;
  [[nodiscard]] virtual AccessibilityRole AccessibleRole() const = 0;
  [[nodiscard]] virtual AccessibilityRect AccessibleLocalRect() const = 0;
  [[nodiscard]] virtual AccessibilityState AccessibleState() const = 0;
};

/*
 * Specialization of GlAccessibleInterface to bridge an OpenGl child element and its QtWidget
 * parent, providing a default implementation for all methods.
 */
class GlAccessibilityBridge : public GlAccessibleInterface {
 public:
  [[nodiscard]] virtual int AccessibleChildCount() const override { return 0; }
  [[nodiscard]] virtual const GlAccessibleInterface* AccessibleChild(int /*index*/) const override {
    return nullptr;
  }
  [[nodiscard]] virtual const GlAccessibleInterface* AccessibleParent() const override {
    return nullptr;
  }

  // The methods below are usually not used and are instead handled by the QtWidget.
  // See the implementation of OpenGlWidgetAccessible in AccessibilityAdapter.cpp!
  [[nodiscard]] virtual std::string AccessibleName() const override { return ""; };
  [[nodiscard]] virtual AccessibilityRole AccessibleRole() const override {
    return AccessibilityRole::Grouping;
  }
  [[nodiscard]] virtual AccessibilityRect AccessibleLocalRect() const override {
    return AccessibilityRect();
  }
  [[nodiscard]] virtual AccessibilityState AccessibleState() const override {
    return AccessibilityState::Normal;
  }
};

}  // namespace orbit_gl

#endif