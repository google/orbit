// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "DataViewUtils.h"

namespace orbit_data_views {

std::string FormatShortDatetime(absl::Time time) {
  return absl::FormatTime("%m/%d/%Y %H:%M %p", time, absl::LocalTimeZone());
}

}  // namespace orbit_data_views