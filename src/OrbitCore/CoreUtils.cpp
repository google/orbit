// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CoreUtils.h"

std::string orbit_core::FormatTime(absl::Time time) {
  return absl::FormatTime("%Y_%m_%d_%H_%M_%S", time, absl::LocalTimeZone());
}
