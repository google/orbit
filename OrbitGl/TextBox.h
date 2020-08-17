// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TEXT_BOX_H_
#define ORBIT_GL_TEXT_BOX_H_

#include <cfloat>
#include <cstdint>

#include "Batcher.h"
#include "CoreMath.h"
#include "capture_data.pb.h"

class TextRenderer;

class TextBox {
 public:
  TextBox();
  TextBox(const Vec2& a_Pos, const Vec2& a_Size, const std::string& a_Text,
          const Color& a_Color = Color(128, 128, 128, 128));

  TextBox(const Vec2& a_Pos, const Vec2& a_Size, const Color& a_Color);
  TextBox(const Vec2& a_Pos, const Vec2& a_Size);

  void Draw(Batcher* batcher, TextRenderer& text_renderer, float min_x = -FLT_MAX,
            bool visible = true, bool right_justify = false, bool is_inactive = false,
            unsigned int id = 0xFFFFFFFF, bool is_picking = false, bool is_highlighted = false);

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

  const std::string& GetText() const { return text_; }
  void SetText(const std::string& a_Text) { text_ = a_Text; }

  void SetTimerInfo(const orbit_client_protos::TimerInfo& timer_info) {
    if (timer_info.end() == 0 && timer_info.start() == 0) {
      return;
    }
    timer_info_ = timer_info;
  }
  const orbit_client_protos::TimerInfo& GetTimerInfo() const { return timer_info_; }

  void SetTextY(float a_Y) { m_TextY = a_Y; }

  void SetElapsedTimeTextLength(size_t a_Length) { m_ElapsedTimeTextLength = a_Length; }
  size_t GetElapsedTimeTextLength() const { return m_ElapsedTimeTextLength; }

  inline void SetColor(Color& a_Color) { m_Color = a_Color; }
  inline void SetColor(uint8_t a_R, uint8_t a_G, uint8_t a_B) {
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
  std::string text_;
  Color m_Color;
  orbit_client_protos::TimerInfo timer_info_;
  int m_MainFrameCounter;
  bool m_Selected;
  float m_TextY;
  size_t m_ElapsedTimeTextLength;
};

inline bool TextBox::Intersects(const TextBox& a_Box) {
  for (int i = 0; i < 2; i++) {
    if (m_Max[i] < a_Box.m_Min[i] || m_Min[i] > a_Box.m_Max[i]) {
      return false;
    }
  }

  return true;
}

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
