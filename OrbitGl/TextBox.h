// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TEXT_BOX_H_
#define ORBIT_GL_TEXT_BOX_H_

#include "BaseTypes.h"
#include "Batcher.h"
#include "CoreMath.h"
#include "capture_data.pb.h"

class TextRenderer;

//-----------------------------------------------------------------------------
class TextBox {
 public:
  TextBox();
  TextBox(const Vec2& a_Pos, const Vec2& a_Size, const std::string& a_Text,
          const Color& a_Color = Color(128, 128, 128, 128));

  TextBox(const Vec2& a_Pos, const Vec2& a_Size, const Color& a_Color);
  TextBox(const Vec2& a_Pos, const Vec2& a_Size);

  ~TextBox();

  void Draw(Batcher* batcher, TextRenderer& a_TextRenderer,
            float a_MinX = -FLT_MAX, bool a_Visible = true,
            bool a_RightJustify = false, bool a_IsInactive = false,
            unsigned int a_ID = 0xFFFFFFFF, bool a_IsPicking = false,
            bool a_IsHighlighted = false);

  void SetSize(const Vec2& a_Size) {
    m_Size = a_Size;
    Update();
  }
  void SetSizeX(float X) {
    m_Size[0] = X;
    Update();
  }
  void SetSizeY(float Y) {
    m_Size[1] = Y;
    Update();
  }

  void SetPos(const Vec2& a_Pos) {
    m_Pos = a_Pos;
    Update();
  }
  void SetPosX(float X) {
    m_Pos[0] = X;
    Update();
  }
  void SetPosY(float Y) {
    m_Pos[1] = Y;
    Update();
  }

  const Vec2& GetSize() const { return m_Size; }
  float GetSizeX() const { return m_Size[0]; }
  float GetSizeY() const { return m_Size[1]; }

  const Vec2& GetPos() const { return m_Pos; }
  float GetPosX() const { return m_Pos[0]; }
  float GetPosY() const { return m_Pos[1]; }

  float GetMaxX() const { return m_Max[0]; }
  float GetMaxY() const { return m_Max[1]; }

  Vec2 GetMin() const { return m_Min; }
  Vec2 GetMax() const { return m_Max; }

  const std::string& GetText() const { return m_Text; }
  void SetText(const std::string& a_Text) { m_Text = a_Text; }

  void SetTimerInfo(const orbit_client_protos::TimerInfo& timer_info) {
    timer_info_ = timer_info;
  }
  const orbit_client_protos::TimerInfo& GetTimerInfo() const {
    return timer_info_;
  }

  void SetTextY(float a_Y) { m_TextY = a_Y; }

  void SetElapsedTimeTextLength(size_t a_Length) {
    m_ElapsedTimeTextLength = a_Length;
  }
  size_t GetElapsedTimeTextLength() const { return m_ElapsedTimeTextLength; }

  inline void SetColor(Color& a_Color) { m_Color = a_Color; }
  inline void SetColor(UCHAR a_R, UCHAR a_G, UCHAR a_B) {
    m_Color[0] = a_R;
    m_Color[1] = a_G;
    m_Color[2] = a_B;
  }
  inline Color GetColor() { return m_Color; }

  float GetScreenSize(const TextRenderer& a_TextRenderer);

  inline bool Intersects(const TextBox& a_Box);
  inline void Expand(const TextBox& a_Box);

  int GetMainFrameCounter() const { return m_MainFrameCounter; }
  void SetMainFrameCounter(int a_Counter) { m_MainFrameCounter = a_Counter; }

  void ToggleSelect() { m_Selected = !m_Selected; }
  void SetSelected(bool a_Select) { m_Selected = a_Select; }

 protected:
  void Update();

 protected:
  Vec2 m_Pos;
  Vec2 m_Size;
  Vec2 m_Min;
  Vec2 m_Max;
  std::string m_Text;
  Color m_Color;
  orbit_client_protos::TimerInfo timer_info_;
  int m_MainFrameCounter;
  bool m_Selected;
  float m_TextY;
  size_t m_ElapsedTimeTextLength;
};

//-----------------------------------------------------------------------------
inline bool TextBox::Intersects(const TextBox& a_Box) {
  for (int i = 0; i < 2; i++) {
    if (m_Max[i] < a_Box.m_Min[i] || m_Min[i] > a_Box.m_Max[i]) {
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
inline void TextBox::Expand(const TextBox& a_Box) {
  for (int i = 0; i < 2; i++) {
    if (a_Box.m_Min[i] < m_Min[i]) {
      m_Min[i] = a_Box.m_Min[i];
    }

    if (a_Box.m_Max[i] > m_Max[i]) {
      m_Max[i] = a_Box.m_Max[i];
    }
  }
}

#endif  // ORBIT_GL_TEXT_BOX_H_
