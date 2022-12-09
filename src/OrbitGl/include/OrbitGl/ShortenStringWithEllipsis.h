// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SHORTEN_STRING_WITH_ELLIPSIS_H_
#define ORBIT_GL_SHORTEN_STRING_WITH_ELLIPSIS_H_

#include <string>

enum class EllipsisPosition { kMiddle };

namespace orbit_gl {

inline std::string ShortenStringWithEllipsis(std::string_view text, size_t max_len,
                                             EllipsisPosition /*pos*/ = EllipsisPosition::kMiddle) {
  // Parameter `pos` is mainly here to indicate how the function works, and to be potentially
  // extended later.

  constexpr const size_t kNumCharsEllipsis = 3;

  if (max_len <= kNumCharsEllipsis) {
    return text.length() <= kNumCharsEllipsis ? std::string(text) : "...";
  }
  if (text.length() <= max_len) {
    return std::string(text);
  }

  const size_t chars_to_cut = text.length() - max_len + kNumCharsEllipsis;
  size_t l = text.length() - chars_to_cut;
  // Integer division by two, rounded up
  if ((l & 0x1) != 0u) {
    l = (l + 1) >> 1;
  } else {
    l = l >> 1;
  }

  const size_t r = l + chars_to_cut;
  return std::string(text.substr(0, l)) + "..." + std::string(text.substr(r));
}

}  // namespace orbit_gl

#endif  // ORBIT_GL_SHORTEN_STRING_WITH_ELLIPSIS_H_
