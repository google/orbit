// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/AccessibleInterfaceProvider.h"

namespace orbit_gl {

orbit_accessibility::AccessibleInterface*
AccessibleInterfaceProvider::GetOrCreateAccessibleInterface() {
  if (accessibility_ == nullptr) {
    accessibility_ = CreateAccessibleInterface();
  }
  return accessibility_.get();
}

}  // namespace orbit_gl