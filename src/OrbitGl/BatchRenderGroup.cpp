// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/BatchRenderGroup.h"

namespace orbit_gl {

[[nodiscard]] BatchRenderGroupState BatchRenderGroupManager::GetGroupState(
    const BatchRenderGroupId& id) const {
  return id_to_state_.contains(id) ? id_to_state_.at(id) : BatchRenderGroupState();
}

void BatchRenderGroupManager::SetGroupState(const BatchRenderGroupId& id,
                                            BatchRenderGroupState state) {
  id_to_state_[id] = state;
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