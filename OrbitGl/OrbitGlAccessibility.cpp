// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGlAccessibility.h"

#include "OrbitBase/Logging.h"

namespace orbit_gl {

GlAccessibleInterfaceRegistry& GlAccessibleInterfaceRegistry::Get() {
  static GlAccessibleInterfaceRegistry registry;
  return registry;
}

GlAccessibleInterfaceRegistry::~GlAccessibleInterfaceRegistry() { CHECK(interfaces_.size() == 0); }

void GlAccessibleInterfaceRegistry::Register(GlAccessibleInterface* iface) {
  if (!interfaces_.contains(iface)) {
    interfaces_.insert(iface);
    if (on_registered_ != nullptr) {
      on_registered_(iface);
    }
  }
}

void GlAccessibleInterfaceRegistry::Unregister(GlAccessibleInterface* iface) {
  CHECK(interfaces_.contains(iface));
  interfaces_.erase(iface);
  if (on_unregistered_ != nullptr) {
    on_unregistered_(iface);
  }
}

void GlAccessibleInterfaceRegistry::OnRegistered(Callback callback) {
  CHECK(on_registered_ == nullptr);
  on_registered_ = callback;
}

void GlAccessibleInterfaceRegistry::OnUnregistered(Callback callback) {
  CHECK(on_unregistered_ == nullptr);
  on_unregistered_ = callback;
}

}  // namespace orbit_gl