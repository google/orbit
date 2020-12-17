// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitAccessibility/AccessibleInterface.h"

#include "OrbitAccessibility/AccessibleInterfaceRegistry.h"

orbit_accessibility::AccessibleInterface::AccessibleInterface() {
  AccessibleInterfaceRegistry::Get().Register(this);
}

orbit_accessibility::AccessibleInterface::~AccessibleInterface() {
  AccessibleInterfaceRegistry::Get().Unregister(this);
}
