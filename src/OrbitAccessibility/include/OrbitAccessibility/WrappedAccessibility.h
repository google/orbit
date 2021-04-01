// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_ACCESSIBILITY_WRAPPED_ACCESSEBILITY_H_
#define ORBIT_ACCESSIBILITY_WRAPPED_ACCESSEBILITY_H_

#include "AccessibleInterface.h"

namespace orbit_accessibility {

/*
 * Interface for classes that do not implement Microsoft's Automation API directly, but provide an
 * `AccessibleInterface`.
 */
class WrappedAccessibility {
 public:
  WrappedAccessibility() = default;
  virtual ~WrappedAccessibility() = default;
  WrappedAccessibility(const WrappedAccessibility& rhs) = delete;
  WrappedAccessibility(WrappedAccessibility&& rhs) = delete;
  WrappedAccessibility& operator=(const WrappedAccessibility& rhs) = delete;
  WrappedAccessibility& operator=(WrappedAccessibility&& rhs) = delete;
  [[nodiscard]] orbit_accessibility::AccessibleInterface* GetOrCreateAccessibleInterface();
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* GetAccessibleInterface() const {
    return accessibility_.get();
  }

 protected:
  [[nodiscard]] virtual std::unique_ptr<AccessibleInterface> CreateAccessibleInterface() = 0;

 private:
  std::unique_ptr<orbit_accessibility::AccessibleInterface> accessibility_;
};

}  // namespace orbit_accessibility

#endif