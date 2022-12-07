// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitAccessibility/AccessibleInterfaceRegistry.h"

#include <utility>

#include "OrbitBase/Logging.h"

namespace orbit_accessibility {

AccessibleInterfaceRegistry& AccessibleInterfaceRegistry::Get() {
  static AccessibleInterfaceRegistry registry;
  return registry;
}

AccessibleInterfaceRegistry::~AccessibleInterfaceRegistry() { ORBIT_CHECK(interfaces_.empty()); }

void AccessibleInterfaceRegistry::Register(AccessibleInterface* iface) {
  if (!interfaces_.contains(iface)) {
    interfaces_.insert(iface);
    if (on_registered_ != nullptr) {
      on_registered_(iface);
    }
  }
}

void AccessibleInterfaceRegistry::Unregister(AccessibleInterface* iface) {
  ORBIT_CHECK(interfaces_.contains(iface));
  interfaces_.erase(iface);
  if (on_unregistered_ != nullptr) {
    on_unregistered_(iface);
  }
}

void AccessibleInterfaceRegistry::SetOnRegisterCallback(Callback callback) {
  ORBIT_CHECK(on_registered_ == nullptr);
  on_registered_ = std::move(callback);
}

void AccessibleInterfaceRegistry::SetOnUnregisterCallback(Callback callback) {
  ORBIT_CHECK(on_unregistered_ == nullptr);
  on_unregistered_ = std::move(callback);
}

}  // namespace orbit_accessibility