// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ThreadBar.h"

#include "AccessibleThreadBar.h"
#include "Track.h"

namespace orbit_gl {

std::unique_ptr<orbit_accessibility::AccessibleInterface> ThreadBar::CreateAccessibleInterface() {
  return std::make_unique<AccessibleThreadBar>(this);
}

}  // namespace orbit_gl
