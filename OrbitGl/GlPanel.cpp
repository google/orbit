// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GlPanel.h"

#include "CaptureWindow.h"

std::unique_ptr<GlPanel> GlPanel::Create(CanvasType canvas_type, uint32_t font_size) {
  switch (canvas_type) {
    case CanvasType::kCaptureWindow:
      return std::make_unique<CaptureWindow>(font_size);
    case CanvasType::kDebug:
      return std::make_unique<GlCanvas>(font_size);
    default:
      ERROR("Unhandled canvas type (%d)", canvas_type);
      return nullptr;
  }
}

GlPanel::GlPanel() {
  m_WindowOffset[0] = 0;
  m_WindowOffset[1] = 0;
  m_MainWindowWidth = 0;
  m_MainWindowHeight = 0;
  m_NeedsRedraw = true;
}

void GlPanel::Initialize() {}

void GlPanel::Resize(int /*a_Width*/, int /*a_Height*/) {}

void GlPanel::Render(int /*a_Width*/, int /*a_Height*/) {}
