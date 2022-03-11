// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_STRING_CONVERSION_H_
#define ORBIT_BASE_STRING_CONVERSION_H_

#include <codecvt>
#include <locale>
#include <string>

namespace orbit_base {

#if WIN32
// On Windows, wchar_t is a 16-bit wide UTF-16 unicode character that we convert to and from UTF-8.
using PlatformWStringConversionFacet = std::codecvt_utf8_utf16<wchar_t>;
#else
// On Linux, wchar_t is a 32-bit wide UTF-32 unicode character that we convert to and from UTF-8.
using PlatformWStringConversionFacet = std::codecvt_utf8<wchar_t>;
#endif

// Convert a wide string to a std::string.
template <typename T>
[[nodiscard]] std::string Narrow(const T& wide_string) {
  static std::wstring_convert<PlatformWStringConversionFacet> converter;
  return converter.to_bytes(wide_string);
}

// Convert a narrow string to a std::wstring.
template <typename T>
[[nodiscard]] std::wstring Widen(const T& string) {
  static std::wstring_convert<PlatformWStringConversionFacet> converter;
  return converter.from_bytes(string);
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_STRING_CONVERSION_H_
