// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gtest/gtest.h>

#ifdef WIN32
#include <Windows.h>
#endif

#include "OrbitBase/ToString.h"

TEST(ToString, CommonTypes) {
  // uint32_t
  ASSERT_EQ(orbit_base::ToString(static_cast<uint32_t>(1234)), "1234");

  // int
  ASSERT_EQ(orbit_base::ToString(static_cast<int>(1)), "1");
  ASSERT_EQ(orbit_base::ToString(-1), "-1");

  // float
  ASSERT_EQ(orbit_base::ToString(1.5f), "1.500000");
  ASSERT_EQ(orbit_base::ToString(-1.5f), "-1.500000");

  // double
  ASSERT_EQ(orbit_base::ToString(1.5), "1.500000");
  ASSERT_EQ(orbit_base::ToString(-1.5), "-1.500000");

  // const char*
  constexpr const char* kTestString = "test_string";
  ASSERT_EQ(orbit_base::ToString(kTestString), kTestString);

  // char*
  std::string test_string = absl::StrFormat("test_string%s", "\0");
  char* test_string_ptr = &test_string[0];
  ASSERT_EQ(orbit_base::ToString(test_string_ptr), test_string);

  // std::string
  ASSERT_EQ(orbit_base::ToString(std::string(kTestString)), kTestString);

  // const wchar_t*
  constexpr const wchar_t* kTestStringW = L"test_string_w";
  ASSERT_EQ(orbit_base::ToString(kTestStringW), "test_string_w");

  // wchar_t*
  std::wstring wide_test_string = std::wstring(L"test_string_w") + L"\0";
  wchar_t* wide_test_string_ptr = &wide_test_string[0];
  ASSERT_EQ(orbit_base::ToString(wide_test_string_ptr), "test_string_w");

  // std::wstring
  ASSERT_EQ(orbit_base::ToString(std::wstring(kTestStringW)), "test_string_w");
}

#ifdef WIN32
TEST(ToString, WindowsString) {
  // Matches ToString(const wchar_t*).
  constexpr const LPCWSTR lpcwstr = L"test_string_const_lpcwstr";
  ASSERT_EQ(orbit_base::ToString(lpcwstr), "test_string_const_lpcwstr");

  // Matches ToString(wchar_t*).
  LPCWSTR non_const_lpcwstr = L"test_string_non_const_lpcwstr";
  ASSERT_EQ(orbit_base::ToString(non_const_lpcwstr), "test_string_non_const_lpcwstr");
}
#endif  // WIN32
