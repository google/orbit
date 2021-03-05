// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitAccessibility/AccessibleInterfaceRegistry.h"

#include "OrbitBase/Logging.h"

namespace orbit_accessibility {

AccessibleInterfaceRegistry& AccessibleInterfaceRegistry::Get() {
  static AccessibleInterfaceRegistry registry;
  return registry;
}

AccessibleInterfaceRegistry::~AccessibleInterfaceRegistry() { CHECK(interfaces_.size() == 0); }

void AccessibleInterfaceRegistry::Register(AccessibleInterface* iface) {
  if (!interfaces_.contains(iface)) {
    interfaces_.insert(iface);
    if (on_registered_ != nullptr) {
      on_registered_(iface);
    }
  }
}

void AccessibleInterfaceRegistry::Unregister(AccessibleInterface* iface) {
  CHECK(interfaces_.contains(iface));
  interfaces_.erase(iface);
  if (on_unregistered_ != nullptr) {
    on_unregistered_(iface);
  }
}

void AccessibleInterfaceRegistry::SetOnRegisterCallback(Callback callback) {
  CHECK(on_registered_ == nullptr);
  on_registered_ = callback;
}

void AccessibleInterfaceRegistry::SetOnUnregisterCallback(Callback callback) {
  CHECK(on_unregistered_ == nullptr);
  on_unregistered_ = callback;
}

}  // namespace orbit_accessibility