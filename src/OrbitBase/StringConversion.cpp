// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/StringConversion.h"

#include <codecvt>
#include <locale>

namespace orbit_base {

#ifdef _WIN32
// On Windows, wchar_t is a 16-bit wide UTF-16 unicode character that we convert to and from UTF-8.
using PlatformWStringConversionFacet = std::codecvt_utf8_utf16<wchar_t>;
#else
// On Linux, wchar_t is a 32-bit wide UTF-32 unicode character that we convert to and from UTF-8.
using PlatformWStringConversionFacet = std::codecvt_utf8<wchar_t>;
#endif

std::string ToStdString(std::wstring_view wide_string) {
  std::wstring_convert<PlatformWStringConversionFacet> converter;
  return converter.to_bytes(wide_string.data(), wide_string.data() + wide_string.length());
}

std::wstring ToStdWString(std::string_view string) {
  std::wstring_convert<PlatformWStringConversionFacet> converter;
  return converter.from_bytes(string.data(), string.data() + string.length());
}

}  // namespace orbit_base
