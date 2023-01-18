// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TEXT_RENDERER_H_
#define ORBIT_GL_TEXT_RENDERER_H_

#include "OrbitGl/BatchRenderGroup.h"
#include "OrbitGl/TextRendererInterface.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

class TextRenderer : public TextRendererInterface {
 public:
  explicit TextRenderer(BatchRenderGroupManager* manager) : manager_(manager){};

  void SetViewport(Viewport* viewport) { viewport_ = viewport; }

  void PushTranslation(float x, float y, float z = 0.f) { translations_.PushTranslation(x, y, z); }
  void PopTranslation() { translations_.PopTranslation(); }

  void SetCurrentRenderGroup(const BatchRenderGroupId& render_group) override {
    current_render_group_ = render_group;
  }
  [[nodiscard]] BatchRenderGroupId GetCurrentRenderGroup() const override {
    return current_render_group_;
  }

  [[nodiscard]] BatchRenderGroupManager* GetRenderGroupManager() { return manager_; }

 protected:
  Viewport* viewport_ = nullptr;

  TranslationStack translations_;
  BatchRenderGroupId current_render_group_;
  BatchRenderGroupManager* manager_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TEXT_RENDERER_H_
