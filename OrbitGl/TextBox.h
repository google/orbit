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

  void SetSize(const Vec2& a_Size) { size_ = a_Size; }

  void SetPos(const Vec2& a_Pos) { pos_ = a_Pos; }

  const Vec2& GetSize() const { return size_; }
  const Vec2& GetPos() const { return pos_; }

  const std::string& GetText() const { return text_; }
  void SetText(std::string_view text) { text_ = text; }

  void SetTimerInfo(const orbit_client_protos::TimerInfo& timer_info) {
    if (timer_info.end() == 0 && timer_info.start() == 0) {
      return;
    }
    timer_info_ = timer_info;
  }
  const orbit_client_protos::TimerInfo& GetTimerInfo() const { return timer_info_; }

  void SetElapsedTimeTextLength(size_t length) { elapsed_time_text_length_ = length; }
  size_t GetElapsedTimeTextLength() const { return elapsed_time_text_length_; }

 protected:
  Vec2 pos_;
  Vec2 size_;
  std::string text_;
  Color color_;
  orbit_client_protos::TimerInfo timer_info_;
  int main_frame_counter_;
  float text_y_;
  size_t elapsed_time_text_length_;
};

#endif  // ORBIT_GL_TEXT_BOX_H_
