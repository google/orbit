// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/BatchRenderGroup.h"

namespace orbit_gl {

[[nodiscard]] BatchRenderGroupState BatchRenderGroupStateManager::GetGroupState(
    const std::string& group_name) const {
  return group_name_to_state_.contains(group_name) ? group_name_to_state_.at(group_name)
                                                   : BatchRenderGroupState();
}

void BatchRenderGroupStateManager::SetGroupState(const std::string& group_name,
                                                 BatchRenderGroupState state) {
  group_name_to_state_[group_name] = state;
}

const std::string BatchRenderGroupId::kGlobalGroup = "global";

bool operator==(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return lhs.layer == rhs.layer && lhs.name == rhs.name;
}

bool operator!=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return !(lhs == rhs);
}

bool operator<(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return lhs.layer < rhs.layer || (lhs.layer == rhs.layer && lhs.name < rhs.name);
}

bool operator<=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return lhs < rhs || lhs == rhs;
}

bool operator>(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return !(lhs <= rhs);
}

bool operator>=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs) {
  return !(lhs < rhs);
}
}  // namespace orbit_gl