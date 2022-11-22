// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <string>

#include "OrbitBase/StringConversion.h"

namespace {

constexpr const wchar_t* kAsciiWideString = L"Ascii string";
constexpr const char* kAsciiString = "Ascii string";
constexpr const wchar_t* kUnicodeWideString = L"CaféSchloß☕🏰🚀😎🐨😁";
constexpr const char* kUnicodeString = "CaféSchloß☕🏰🚀😎🐨😁";
const std::string kEmptyString;
const std::wstring kEmptyWideString;

}  // namespace

TEST(StringConversion, NarrowAscii) {
  // const wchar_t* to std::string.
  EXPECT_EQ(orbit_base::ToStdString(kAsciiWideString), kAsciiString);

  // std::wstring to std::string.
  EXPECT_EQ(orbit_base::ToStdString(std::wstring(kAsciiWideString)), kAsciiString);
}

TEST(StringConversion, WidenAscii) {
  // const char* to std::wstring.
  EXPECT_EQ(orbit_base::ToStdWString(kAsciiString), kAsciiWideString);

  // std::string to std::wstring.
  EXPECT_EQ(orbit_base::ToStdWString(std::string(kAsciiString)), kAsciiWideString);
}

TEST(StringConversion, NarrowUnicode) {
  // const wchar_t* to std::string.
  EXPECT_EQ(orbit_base::ToStdString(kUnicodeWideString), kUnicodeString);

  // std::wstring to std::string.
  EXPECT_EQ(orbit_base::ToStdString(std::wstring(kUnicodeWideString)), kUnicodeString);
}

TEST(StringConversion, WidenUnicode) {
  // const char* to std::wstring.
  EXPECT_EQ(orbit_base::ToStdWString(kUnicodeString), kUnicodeWideString);

  // std::string to std::wstring.
  EXPECT_EQ(orbit_base::ToStdWString(std::string(kUnicodeString)), kUnicodeWideString);
}

TEST(StringConversion, NarrowEmpty) {
  // const wchar_t* to std::string.
  EXPECT_EQ(orbit_base::ToStdString(L""), "");

  // std::wstring to std::string.
  EXPECT_EQ(orbit_base::ToStdString(kEmptyWideString), "");
}

TEST(StringConversion, WidenEmpty) {
  // const char* to std::wstring.
  EXPECT_EQ(orbit_base::ToStdWString(""), L"");

  // std::string to std::wstring.
  EXPECT_EQ(orbit_base::ToStdWString(kEmptyString), L"");
}
