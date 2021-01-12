// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TextBox.h"

TextBox::TextBox() : pos_(Vec2::Zero()), size_(Vec2(100.f, 10.f)), elapsed_time_text_length_(0) {}

TextBox::TextBox(const Vec2& pos, const Vec2& size, const std::string& text)
    : pos_(pos), size_(size), text_(text), elapsed_time_text_length_(0) {}