// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TEXT_BOX_H_
#define ORBIT_GL_TEXT_BOX_H_

#include <utility>

#include "CoreMath.h"
#include "capture_data.pb.h"

class TextBox {
 public:
  TextBox() = default;
  explicit TextBox(orbit_client_protos::TimerInfo timer_info)
      : timer_info_{std::move(timer_info)} {}
  TextBox(orbit_client_protos::TimerInfo timer_info, const Vec2& pos, const Vec2& size,
          std::string text = "")
      : timer_info_{std::move(timer_info)}, pos_{pos}, size_{size}, text_{std::move(text)} {}

  // Delete the copy- and move-assignment operators, while keeping the copy- and move- constructors.
  // This is so that an element in TimerChain cannot just be re-assigned, which would break the
  // invariance on TimerBlock::min_timestamp_ and max_timestamp_.
  TextBox(const TextBox& other) = default;
  TextBox& operator=(const TextBox& other) = delete;
  TextBox(TextBox&& other) = default;
  TextBox& operator=(TextBox&& other) = delete;

  void SetSize(const Vec2& size) { size_ = size; }
  void SetPos(const Vec2& pos) { pos_ = pos; }

  const Vec2& GetSize() const { return size_; }
  const Vec2& GetPos() const { return pos_; }

  const std::string& GetText() const { return text_; }
  void SetText(std::string text) { text_ = std::move(text); }

  const orbit_client_protos::TimerInfo& GetTimerInfo() const { return timer_info_; }

  void SetElapsedTimeTextLength(size_t length) { elapsed_time_text_length_ = length; }
  size_t GetElapsedTimeTextLength() const { return elapsed_time_text_length_; }

  // Start() and End() are required in order to be used as node in a ScopeTree.
  uint64_t Start() const { return timer_info_.start(); }
  uint64_t End() const { return timer_info_.end(); }
  uint64_t Duration() const { return End() - Start(); }

 protected:
  orbit_client_protos::TimerInfo timer_info_;
  Vec2 pos_ = Vec2::Zero();
  Vec2 size_ = Vec2::Zero();
  std::string text_;
  size_t elapsed_time_text_length_ = 0;
};

#endif  // ORBIT_GL_TEXT_BOX_H_
