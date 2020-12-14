// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <functional>

#include "AccessibleObjectFake.h"
#include "OrbitGlAccessibility.h"

namespace orbit_gl {

TEST(GlAccessibleInterfaceRegistry, Management) {
  auto impl = std::make_unique<AccessibleObjectFake>(nullptr);
  auto impl_ptr = impl.get();
  EXPECT_TRUE(GlAccessibleInterfaceRegistry::Get().Exists(impl_ptr));
  impl.reset();
  EXPECT_FALSE(GlAccessibleInterfaceRegistry::Get().Exists(impl_ptr));
}

TEST(AccessibilityRegistry, Callback) {
  bool registered = false;

  auto registered_callback = [&registered](GlAccessibleInterface* /*iface*/) { registered = true; };

  auto unregistered_callback = [&registered](GlAccessibleInterface* /*iface*/) {
    registered = false;
  };

  GlAccessibleInterfaceRegistry::Get().SetOnRegisterCallback(registered_callback);
  GlAccessibleInterfaceRegistry::Get().SetOnUnregisterCallback(unregistered_callback);

  auto impl = std::make_unique<AccessibleObjectFake>(nullptr);
  EXPECT_TRUE(registered);
  impl.reset();
  EXPECT_FALSE(registered);
}

}  // namespace orbit_gl