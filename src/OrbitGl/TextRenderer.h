// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TEXT_RENDERER_H_
#define ORBIT_GL_TEXT_RENDERER_H_

#include "TextRendererInterface.h"
#include "Viewport.h"

namespace orbit_gl {

class TextRenderer : public TextRendererInterface {
 public:
  explicit TextRenderer() {}
  void SetViewport(Viewport* viewport) { viewport_ = viewport; }

  void PushTranslation(float x, float y, float z = 0.f) { translations_.PushTranslation(x, y, z); }
  void PopTranslation() { translations_.PopTranslation(); }

  static void SetDrawOutline(bool value) { draw_outline_ = value; }

 protected:
  Viewport* viewport_ = nullptr;
  inline static bool draw_outline_ = false;

  TranslationStack translations_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TEXT_RENDERER_H_
