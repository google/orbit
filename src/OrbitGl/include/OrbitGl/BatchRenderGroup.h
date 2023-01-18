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

class BatchRenderGroupStateManager;

struct BatchRenderGroupId {
  static const std::string kGlobalGroup;

  std::string name = kGlobalGroup;
  float layer = 0;

  friend bool operator==(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
  friend bool operator!=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
  friend bool operator<(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
  friend bool operator<=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
  friend bool operator>(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);
  friend bool operator>=(const BatchRenderGroupId& lhs, const BatchRenderGroupId& rhs);

  BatchRenderGroupId& operator=(const BatchRenderGroupId& rhs) = default;
  BatchRenderGroupId(const BatchRenderGroupId& rhs) = default;
  BatchRenderGroupId(BatchRenderGroupId&& rhs) = default;

  explicit BatchRenderGroupId(float layer = 0, std::string name = kGlobalGroup)
      : name(std::move(name)), layer(layer) {}
};

}  // namespace orbit_gl

namespace std {
template <>
struct hash<orbit_gl::BatchRenderGroupId> {
  size_t operator()(const orbit_gl::BatchRenderGroupId& obj) const {
    // "Inspired" by "hash_combine" from boost
    size_t seed = std::hash<float>()(obj.layer);
    seed ^= hash<std::string>()(obj.name) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
  }
};
}  // namespace std

namespace orbit_gl {

struct BatchRenderGroupState {
  StencilConfig stencil;
};

class BatchRenderGroupStateManager {
 public:
  [[nodiscard]] BatchRenderGroupState GetGroupState(const std::string& group_name) const;
  void SetGroupState(const std::string& group_name, BatchRenderGroupState state);

 private:
  absl::flat_hash_map<std::string, BatchRenderGroupState> group_name_to_state_;
};

}  // namespace orbit_gl

#endif