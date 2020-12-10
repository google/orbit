// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <functional>

#include "AccessibilityInterfaceMock.h"

namespace orbit_gl {

TEST(AccessibilityRegistry, Management) {
  TestA11yImpl* impl = new TestA11yImpl(nullptr);
  EXPECT_TRUE(GlAccessibleInterfaceRegistry::Get().Exists(impl));
  delete impl;
  EXPECT_FALSE(GlAccessibleInterfaceRegistry::Get().Exists(impl));
}

TEST(AccessibilityRegistry, Callback) {
  bool registered = false;

  auto registered_callback = [&registered](GlAccessibleInterface* iface) { registered = true; };

  auto unregistered_callback = [&registered](GlAccessibleInterface* iface) { registered = false; };

  GlAccessibleInterfaceRegistry::Get().OnRegistered(registered_callback);
  GlAccessibleInterfaceRegistry::Get().OnUnregistered(unregistered_callback);

  TestA11yImpl* impl = new TestA11yImpl(nullptr);
  EXPECT_TRUE(registered);
  delete impl;
  EXPECT_FALSE(registered);
}

}  // namespace orbit_gl