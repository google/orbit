// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __ORBIT_GL_BATCH_RENDER_GROUP_H__
#define __ORBIT_GL_BATCH_RENDER_GROUP_H__

#include <functional>
#include <string>

#include "absl/container/flat_hash_map.h"

namespace orbit_gl {

struct StencilConfig {
  bool enabled = false;
  std::array<float, 2> pos = {0, 0};
  std::array<float, 2> size = {0, 0};
};

struct BatchRenderGroupId {
  static const std::string kGlobalGroup;

  std::string name = kGlobalGroup;
  float layer;

  explicit BatchRenderGroupId(const std::string& name, float layer);
  explicit BatchRenderGroupId(float layer = 0) : BatchRenderGroupId(kGlobalGroup, layer){};
};

bool operator==(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
bool operator!=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
bool operator<(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
bool operator<=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
bool operator>(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
bool operator>=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);

struct BatchRenderGroupState {
  StencilConfig stencil;
};

namespace BatchRenderGroupManager {
void ResetOrdering();
void TouchId(const BatchRenderGroupId& id);

[[nodiscard]] BatchRenderGroupState GetGroupState(const BatchRenderGroupId& id);
void SetGroupState(const BatchRenderGroupId& id, BatchRenderGroupState state);
}  // namespace BatchRenderGroupManager
}  // namespace orbit_gl

namespace std {
template <>
struct hash<orbit_gl::BatchRenderGroupId> {
  size_t operator()(const orbit_gl::BatchRenderGroupId& obj) const {
    return hash<float>()(obj.layer) ^ hash<std::string>()(obj.name);
  }
};
}  // namespace std

#endif