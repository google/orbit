// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TO_STRING_H_
#define ORBIT_BASE_TO_STRING_H_

#include <algorithm>
#include <string>

// Wrapper around std::to_string which adds support for strings.

namespace orbit_base {

template <typename T>
[[nodiscard]] inline std::string ToString(const T& value) {
  return std::to_string(value);
}

[[nodiscard]] inline std::string ToString(const wchar_t* value, size_t length) {
  std::string result(length, 0);
  std::transform(value, value + length, result.begin(),
                 [](wchar_t c) { return static_cast<char>(c); });
  return result;
}

[[nodiscard]] inline std::string ToString(wchar_t* value) { return ToString(value, wcslen(value)); }
[[nodiscard]] inline std::string ToString(const wchar_t* value) {
  return ToString(value, wcslen(value));
}
[[nodiscard]] inline std::string ToString(const std::wstring& value) {
  return ToString(value.c_str(), value.size());
}

[[nodiscard]] inline std::string ToString(char* value) { return std::string(value); }
[[nodiscard]] inline std::string ToString(const char* value) { return std::string(value); }
[[nodiscard]] inline std::string ToString(const std::string& value) { return value; }

}  // namespace orbit_base

#endif  // ORBIT_BASE_TO_STRING_H_
