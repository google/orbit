// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TEXT_BOX_H_
#define ORBIT_GL_TEXT_BOX_H_

#include <utility>

#include "CoreMath.h"
#include "capture_data.pb.h"

class TextRenderer;

class TextBox {
 public:
  TextBox() : pos_(Vec2::Zero()), size_(Vec2::Zero()) {}
  TextBox(const Vec2& pos, const Vec2& size, std::string text = "")
      : pos_(pos), size_(size), text_(std::move(text)) {}

  void SetSize(const Vec2& size) { size_ = size; }
  void SetPos(const Vec2& pos) { pos_ = pos; }

  const Vec2& GetSize() const { return size_; }
  const Vec2& GetPos() const { return pos_; }

  const std::string& GetText() const { return text_; }
  void SetText(std::string text) { text_ = std::move(text); }

  void SetTimerInfo(const orbit_client_protos::TimerInfo& timer_info) {
    if (timer_info.end() == 0 && timer_info.start() == 0) {
      return;
    }
    timer_info_ = timer_info;
  }
  const orbit_client_protos::TimerInfo& GetTimerInfo() const { return timer_info_; }
  orbit_client_protos::TimerInfo& GetMutableTimerInfo() { return timer_info_; }

  void SetElapsedTimeTextLength(size_t length) { elapsed_time_text_length_ = length; }
  size_t GetElapsedTimeTextLength() const { return elapsed_time_text_length_; }

  // Start() and End() are required in order to be used as node in a ScopeTree.
  uint64_t Start() const { return timer_info_.start(); }
  uint64_t End() const { return timer_info_.end(); }
  uint64_t Duration() const { return End() - Start(); }

 protected:
  Vec2 pos_;
  Vec2 size_;
  std::string text_;
  orbit_client_protos::TimerInfo timer_info_;
  size_t elapsed_time_text_length_ = 0;
};

#endif  // ORBIT_GL_TEXT_BOX_H_
