// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/BatchRenderGroup.h"

#include "OrbitBase/Logging.h"

namespace orbit_gl {

inline bool BatchRenderGroupManager::HasValidCreationIndex(const BatchRenderGroupId& id) const {
  return id_to_touch_index_.contains(id) && (id_to_touch_index_.at(id) >= frame_start_index_);
}

inline uint64_t BatchRenderGroupManager::GetCreationOrderIndex(const BatchRenderGroupId& id) const {
  uint64_t index = id_to_touch_index_.contains(id) ? id_to_touch_index_.at(id) : frame_start_index_;
  if (index < frame_start_index_) {
    index = frame_start_index_;
  }

  return index;
}

void BatchRenderGroupManager::ResetOrdering() { frame_start_index_ = ++current_touch_index_; }

void BatchRenderGroupManager::TouchId(const BatchRenderGroupId& id) {
  if (!HasValidCreationIndex(id)) {
    id_to_touch_index_[id] = ++current_touch_index_;
  }
}

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

bool BatchRenderGroupIdComparator::operator()(const BatchRenderGroupId& lhs,
                                              const BatchRenderGroupId& rhs) {
  ORBIT_CHECK(lhs.manager_ == rhs.manager_ && lhs.manager_ == manager_);

  if (lhs.layer != rhs.layer) return lhs.layer < rhs.layer;

  uint64_t lhs_creation = manager_->GetCreationOrderIndex(lhs);
  uint64_t rhs_creation = manager_->GetCreationOrderIndex(rhs);
  return lhs_creation < rhs_creation;
}

}  // namespace orbit_gl