// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_ACCESSIBLE_INTERFACE_PROVIDER_H_
#define ORBIT_GL_ACCESSIBLE_INTERFACE_PROVIDER_H_

#include <memory>

#include "OrbitAccessibility/AccessibleInterface.h"

namespace orbit_gl {

/* Base class to provide accessibility in the capture window. */
class AccessibleInterfaceProvider {
 public:
  virtual ~AccessibleInterfaceProvider() = default;

  [[nodiscard]] orbit_accessibility::AccessibleInterface* GetOrCreateAccessibleInterface();

 private:
  [[nodiscard]] virtual std::unique_ptr<orbit_accessibility::AccessibleInterface>
  CreateAccessibleInterface() = 0;
  std::unique_ptr<orbit_accessibility::AccessibleInterface> accessibility_;
};

}  // namespace orbit_gl

#endif
