// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STRING_CONVERSION_H_
#define ORBIT_BASE_STRING_CONVERSION_H_

#include <string>
#include <string_view>

namespace orbit_base {

// Convert a wide string to a std::string.
[[nodiscard]] std::string ToStdString(std::wstring_view wide_string);

// Convert a narrow string to a std::wstring.
[[nodiscard]] std::wstring ToStdWString(std::string_view string);

}  // namespace orbit_base

#endif  // ORBIT_BASE_STRING_CONVERSION_H_
