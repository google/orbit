// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TextBox.h"

#include "App.h"
#include "Capture.h"
#include "FunctionUtils.h"
#include "GlCanvas.h"
#include "OpenGl.h"
#include "Params.h"
#include "TextRenderer.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::FunctionInfo;
using orbit_client_protos::TimerInfo;

TextBox::TextBox()
    : m_Pos(Vec2::Zero()),
      m_Size(Vec2(100.f, 10.f)),
      m_MainFrameCounter(-1),
      m_Selected(false),
      m_TextY(FLT_MAX),
      m_ElapsedTimeTextLength(0) {
  Update();
}

TextBox::TextBox(const Vec2& a_Pos, const Vec2& a_Size, const std::string& a_Text,
                 const Color& a_Color)
    : m_Pos(a_Pos),
      m_Size(a_Size),
      text_(a_Text),
      m_Color(a_Color),
      m_MainFrameCounter(-1),
      m_Selected(false),
      m_ElapsedTimeTextLength(0) {
  Update();
}

TextBox::TextBox(const Vec2& a_Pos, const Vec2& a_Size, const Color& a_Color)
    : m_Pos(a_Pos),
      m_Size(a_Size),
      m_Color(a_Color),
      m_MainFrameCounter(-1),
      m_Selected(false),
      m_ElapsedTimeTextLength(0) {
  Update();
}

TextBox::TextBox(const Vec2& a_Pos, const Vec2& a_Size)
    : m_Pos(a_Pos),
      m_Size(a_Size),
      m_MainFrameCounter(-1),
      m_Selected(false),
      m_ElapsedTimeTextLength(0) {
  Update();
}

void TextBox::Update() {
  m_Min = m_Pos;
  m_Max = m_Pos + Vec2(std::abs(m_Size[0]), std::abs(m_Size[1]));
}

float TextBox::GetScreenSize(const TextRenderer& a_TextRenderer) {
  float worldWidth = a_TextRenderer.GetSceneBox().m_Size[0];
  float screenSize = a_TextRenderer.GetCanvas()->getWidth();

  return (m_Size[0] / worldWidth) * screenSize;
}

void TextBox::Draw(Batcher* batcher, TextRenderer& text_renderer, float min_x, bool visible,
                   bool right_justify, bool is_inactive, unsigned int id, bool is_picking,
                   bool is_highlighted) {
  bool is_core_activity = timer_info_.type() == TimerInfo::kCoreActivity;
  bool is_same_thread_id_as_selected =
      is_core_activity && timer_info_.thread_id() == GOrbitApp->selected_thread_id();

  if (GOrbitApp->selected_thread_id() != 0 && is_core_activity && !is_same_thread_id_as_selected) {
    is_inactive = true;
  }

  static unsigned char g = 100;
  Color grey(g, g, g, 255);
  static Color selection_color(0, 128, 255, 255);

  Color col = is_same_thread_id_as_selected ? m_Color : is_inactive ? grey : m_Color;

  if (this == Capture::GSelectedTextBox) {
    col = selection_color;
  }

  float z = is_highlighted
                ? GlCanvas::Z_VALUE_CONTEXT_SWITCH
                : is_inactive ? GlCanvas::Z_VALUE_BOX_INACTIVE : GlCanvas::Z_VALUE_BOX_ACTIVE;

  Color color = col;
  if (is_picking) {
    memcpy(&color[0], &id, sizeof(unsigned int));
    color[3] = 255;
  }

  if (visible) {
    Box box(m_Pos, m_Size, z);
    // TODO: This should be pickable??
    batcher->AddBox(box, color);

    static Color a_color(255, 255, 255, 255);

    float pos_x = std::max(m_Pos[0], min_x);
    if (right_justify) {
      pos_x += m_Size[0];
    }

    float max_size = m_Pos[0] + m_Size[0] - pos_x;

    const FunctionInfo* func =
        Capture::capture_data_.GetSelectedFunction(timer_info_.function_address());
    std::string function_name = func != nullptr ? FunctionUtils::GetDisplayName(*func) : "";
    std::string text = absl::StrFormat("%s %s", function_name, text_.c_str());

    if (!is_picking && !is_core_activity) {
      text_renderer.AddText(text.c_str(), pos_x, m_TextY == FLT_MAX ? m_Pos[1] + 1.f : m_TextY,
                            GlCanvas::Z_VALUE_TEXT, a_color, max_size, right_justify);
    }
  }

  // TODO: This should be pickable??
  batcher->AddVerticalLine(m_Pos, m_Size[1], z, grey);
}
