// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_ACCESSIBILITY_ACCESSIBLE_INTERFACE_REGISTRY_H_
#define ORBIT_ACCESSIBILITY_ACCESSIBLE_INTERFACE_REGISTRY_H_

#include <absl/container/flat_hash_set.h>

#include <functional>

#include "OrbitAccessibility/AccessibleInterface.h"

namespace orbit_accessibility {

/* Singleton class, keeps track of created and destroyed AccessibleInterface instances.
 * This is required to respond to the destruction of interfaces outside of OrbitGl.
 *
 * orbit_qt::AccessibilityAdapter will register itself for the "SetOnUnregisterCallback" callback.
 * See OrbitQt/AccessibilityAdapter.h for more documentation.
 */
class AccessibleInterfaceRegistry {
 public:
  using Callback = std::function<void(AccessibleInterface*)>;

  void Register(AccessibleInterface* iface);
  void Unregister(AccessibleInterface* iface);

  void SetOnRegisterCallback(Callback callback);
  void SetOnUnregisterCallback(Callback callback);

  [[nodiscard]] bool Exists(AccessibleInterface* iface) { return interfaces_.contains(iface); }

  static AccessibleInterfaceRegistry& Get();

 private:
  AccessibleInterfaceRegistry() = default;
  ~AccessibleInterfaceRegistry();

  absl::flat_hash_set<AccessibleInterface*> interfaces_;
  Callback on_registered_ = nullptr;
  Callback on_unregistered_ = nullptr;
};

}  // namespace orbit_accessibility

#endif