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
    : pos_(Vec2::Zero()),
      size_(Vec2(100.f, 10.f)),
      main_frame_counter_(-1),
      text_y_(FLT_MAX),
      elapsed_time_text_length_(0) {}

TextBox::TextBox(const Vec2& a_Pos, const Vec2& a_Size, const std::string& a_Text,
                 const Color& a_Color)
    : pos_(a_Pos),
      size_(a_Size),
      text_(a_Text),
      color_(a_Color),
      text_y_(FLT_MAX),
      main_frame_counter_(-1),
      elapsed_time_text_length_(0) {}

TextBox::TextBox(const Vec2& a_Pos, const Vec2& a_Size, const Color& a_Color)
    : pos_(a_Pos),
      size_(a_Size),
      color_(a_Color),
      text_y_(FLT_MAX),
      main_frame_counter_(-1),
      elapsed_time_text_length_(0) {}

TextBox::TextBox(const Vec2& a_Pos, const Vec2& a_Size)
    : pos_(a_Pos),
      size_(a_Size),
      main_frame_counter_(-1),
      elapsed_time_text_length_(0),
      text_y_(FLT_MAX) {}