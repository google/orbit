// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_DATA_VIEW_UTILS_H_
#define DATA_VIEWS_DATA_VIEW_UTILS_H_

#include <absl/time/time.h>

#include <string>

namespace orbit_data_views {
std::string FormatShortDatetime(absl::Time time);
}  // namespace orbit_data_views

#endif  // DATA_VIEWS_DATA_VIEW_UTILS_H_