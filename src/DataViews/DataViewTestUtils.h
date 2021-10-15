// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_
#define DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_

#include <string>
#include <vector>

namespace orbit_data_views {
void CheckSingleAction(const std::vector<std::string>& context_menu, const std::string& action,
                       bool enable_action);
}  // namespace orbit_data_views

#endif  // DATA_VIEWS_DATA_VIEW_TEST_UTILS_H_