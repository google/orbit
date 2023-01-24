// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_BATCHER_H_
#define ORBIT_GL_BATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <utility>

#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/TranslationStack.h"

namespace orbit_gl {

// Collects primitives to be rendered at a later point in time.
//
// This abstract class extends BatcherInterface by taking care of holding batcher_id and
// translations that will be called by CaptureViewElements.
class Batcher : public BatcherInterface {
 public:
  explicit Batcher(BatcherId batcher_id) : batcher_id_(batcher_id) {}

  [[nodiscard]] BatcherId GetBatcherId() const { return batcher_id_; }

  void PushTranslation(float x, float y, float z = 0.f) { translations_.PushTranslation(x, y, z); }
  void PopTranslation() { translations_.PopTranslation(); }

  struct Statistics {
    size_t reserved_memory = 0;
    uint32_t draw_calls = 0;
    uint32_t stored_layers = 0;
    uint32_t stored_vertices = 0;

    friend bool operator==(const Batcher::Statistics& lhs, const Batcher::Statistics& rhs);
    friend bool operator!=(const Batcher::Statistics& lhs, const Batcher::Statistics& rhs);
  };

  [[nodiscard]] virtual Statistics GetStatistics() const = 0;
  [[nodiscard]] std::string GetCurrentRenderGroupName() const override {
    return current_render_group_.name;
  }
  void SetCurrentRenderGroupName(std::string name) override {
    current_render_group_.name = std::move(name);
  }

 protected:
  orbit_gl::TranslationStack translations_;
  BatchRenderGroupId current_render_group_;

 private:
  BatcherId batcher_id_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_BATCHER_H_
