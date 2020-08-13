// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TextBox.h"

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

TextBox::TextBox(const Vec2& a_Pos, const Vec2& a_Size,
                 const std::string& a_Text, const Color& a_Color)
    : m_Pos(a_Pos),
      m_Size(a_Size),
      m_Text(a_Text),
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

void TextBox::Draw(Batcher* batcher, TextRenderer& a_TextRenderer, float a_MinX,
                   bool a_Visible, bool a_RightJustify, bool isInactive,
                   unsigned int a_ID, bool a_IsPicking, bool a_IsHighlighted) {
  bool isCoreActivity = timer_info_.type() == TimerInfo::kCoreActivity;
  bool isSameThreadIdAsSelected =
      isCoreActivity && timer_info_.thread_id() == Capture::GSelectedThreadId;

  if (Capture::GSelectedThreadId != 0 && isCoreActivity &&
      !isSameThreadIdAsSelected) {
    isInactive = true;
  }

  static unsigned char g = 100;
  Color grey(g, g, g, 255);
  static Color selectionColor(0, 128, 255, 255);

  Color col = isSameThreadIdAsSelected ? m_Color : isInactive ? grey : m_Color;

  if (this == Capture::GSelectedTextBox) {
    col = selectionColor;
  }

  float z = a_IsHighlighted ? GlCanvas::Z_VALUE_CONTEXT_SWITCH
                            : isInactive ? GlCanvas::Z_VALUE_BOX_INACTIVE
                                         : GlCanvas::Z_VALUE_BOX_ACTIVE;

  Color color = col;
  if (a_IsPicking) {
    memcpy(&color[0], &a_ID, sizeof(unsigned int));
    color[3] = 255;
  }

  if (a_Visible) {
    Box box(m_Pos, m_Size, z);
    // TODO: This should be pickable??
    batcher->AddBox(box, color);

    static Color s_Color(255, 255, 255, 255);

    float posX = std::max(m_Pos[0], a_MinX);
    if (a_RightJustify) {
      posX += m_Size[0];
    }

    float maxSize = m_Pos[0] + m_Size[0] - posX;

    FunctionInfo* func =
        Capture::GSelectedFunctionsMap[timer_info_.function_address()];
    std::string text = absl::StrFormat(
        "%s %s", func ? FunctionUtils::GetDisplayName(*func).c_str() : "",
        m_Text.c_str());

    if (!a_IsPicking && !isCoreActivity) {
      a_TextRenderer.AddText(
          text.c_str(), posX, m_TextY == FLT_MAX ? m_Pos[1] + 1.f : m_TextY,
          GlCanvas::Z_VALUE_TEXT, s_Color, maxSize, a_RightJustify);
    }
  }

  // TODO: This should be pickable??
  batcher->AddVerticalLine(m_Pos, m_Size[1], z, grey);
}
