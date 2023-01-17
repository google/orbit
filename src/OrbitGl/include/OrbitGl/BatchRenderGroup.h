// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCH_RENDER_GROUP_H_
#define ORBIT_GL_BATCH_RENDER_GROUP_H_

#include <functional>
#include <string>

#include "absl/container/flat_hash_map.h"

namespace orbit_gl {

struct StencilConfig {
  bool enabled = false;
  std::array<float, 2> pos = {0, 0};
  std::array<float, 2> size = {0, 0};
};

class BatchRenderGroupManager;

struct BatchRenderGroupId {
  static const std::string kGlobalGroup;

  std::string name = kGlobalGroup;
  float layer;

  friend bool operator==(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
  friend bool operator!=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);

  BatchRenderGroupId& operator=(const BatchRenderGroupId& rhs) {
    if (this == &rhs) return *this;

    name = rhs.name;
    layer = rhs.layer;
    return *this;
  }

  BatchRenderGroupId(const BatchRenderGroupId& rhs) = default;

 private:
  friend class BatchRenderGroupManager;
  friend class BatchRenderGroupIdComparator;

  explicit BatchRenderGroupId(const BatchRenderGroupManager* manager, float layer = 0,
                              std::string name = kGlobalGroup)
      : name(std::move(name)), layer(layer), manager_(manager) {}
  const BatchRenderGroupManager* manager_;
};

}  // namespace orbit_gl

namespace std {
template <>
struct hash<orbit_gl::BatchRenderGroupId> {
  size_t operator()(const orbit_gl::BatchRenderGroupId& obj) const {
    return hash<float>()(obj.layer) ^ hash<std::string>()(obj.name);
  }
};
}  // namespace std

namespace orbit_gl {

struct BatchRenderGroupState {
  StencilConfig stencil;
};

class BatchRenderGroupIdComparator {
 public:
  [[nodiscard]] bool operator()(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);

 private:
  friend class BatchRenderGroupManager;

  explicit BatchRenderGroupIdComparator(BatchRenderGroupManager* manager) : manager_(manager) {}

  const BatchRenderGroupManager* manager_;
};

class BatchRenderGroupManager {
 public:
  void ResetOrdering();
  void TouchId(const BatchRenderGroupId& id);

  [[nodiscard]] BatchRenderGroupState GetGroupState(const BatchRenderGroupId& id) const;
  void SetGroupState(const BatchRenderGroupId& id, BatchRenderGroupState state);

  [[nodiscard]] BatchRenderGroupId CreateId(float layer = 0,
                                            std::string name = BatchRenderGroupId::kGlobalGroup) {
    return BatchRenderGroupId(this, layer, name);
  }

  [[nodiscard]] BatchRenderGroupIdComparator CreateComparator() {
    return BatchRenderGroupIdComparator(this);
  }

 private:
  absl::flat_hash_map<BatchRenderGroupId, uint64_t> id_to_touch_index_;
  absl::flat_hash_map<BatchRenderGroupId, BatchRenderGroupState> id_to_state_;
  uint64_t current_touch_index_ = 0;
  uint64_t frame_start_index_ = 0;

  [[nodiscard]] bool HasValidCreationIndex(const BatchRenderGroupId& id) const;
  [[nodiscard]] uint64_t GetCreationOrderIndex(const BatchRenderGroupId& id) const;

  friend class BatchRenderGroupIdComparator;
};

}  // namespace orbit_gl

#endif