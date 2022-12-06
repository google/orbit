// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitAccessibility/AccessibleInterfaceRegistry.h"
#include "OrbitAccessibility/AccessibleObjectFake.h"

namespace orbit_accessibility {

TEST(AccessibleInterfaceRegistry, Management) {
  auto impl = std::make_unique<AccessibleObjectFake>(nullptr);
  auto* impl_ptr = impl.get();
  EXPECT_TRUE(AccessibleInterfaceRegistry::Get().Exists(impl_ptr));
  impl.reset();
  EXPECT_FALSE(AccessibleInterfaceRegistry::Get().Exists(impl_ptr));
}

TEST(AccessibilityRegistry, Callback) {
  bool registered = false;

  auto registered_callback = [&registered](AccessibleInterface* /*iface*/) { registered = true; };

  auto unregistered_callback = [&registered](AccessibleInterface* /*iface*/) {
    registered = false;
  };

  AccessibleInterfaceRegistry::Get().SetOnRegisterCallback(registered_callback);
  AccessibleInterfaceRegistry::Get().SetOnUnregisterCallback(unregistered_callback);

  auto impl = std::make_unique<AccessibleObjectFake>(nullptr);
  EXPECT_TRUE(registered);
  impl.reset();
  EXPECT_FALSE(registered);
}

}  // namespace orbit_accessibility