// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/StringConversion.h"

namespace {

const std::wstring kAsciiWideString = L"Ascii string";
const std::string kAsciiString = "Ascii string";
const std::wstring kUnicodeWideString = L"CaféSchloß🏰🚀😎🐨😁";
const std::string kUnicodeString = "CaféSchloß🏰🚀😎🐨😁";

}  // namespace

TEST(StringConversion, NarrowAscii) {
  // Ascii string.
  EXPECT_EQ(orbit_base::Narrow(kAsciiWideString), kAsciiString);
}

TEST(StringConversion, WidenAscii) {
  // Ascii string.
  EXPECT_EQ(orbit_base::Widen(kAsciiString), kAsciiWideString);
}

TEST(StringConversion, NarrowUnicode) {
  // Unicode string.
  EXPECT_EQ(orbit_base::Narrow(kUnicodeWideString), kUnicodeString);
}

TEST(StringConversion, WidenUnicode) {
  // Unicode string.
  EXPECT_EQ(orbit_base::Widen(kUnicodeString), kUnicodeWideString);
}

TEST(StringConversion, NarrowEmpty) {
  // Empty string.
  EXPECT_EQ(orbit_base::Narrow(L""), "");
}

TEST(StringConversion, WidenEmpty) {
  // Empty string.
  EXPECT_EQ(orbit_base::Widen(""), L"");
}
