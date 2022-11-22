// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <string>
#include <system_error>

#include "OrbitBase/Result.h"

TEST(Result, ErrorMessageCanBeConstructedFromString) {
  ErrorMessage error_message{"Hello World!"};
  EXPECT_EQ(error_message.message(), "Hello World!");
}

TEST(Result, ErrorMessageCanBeConstructedFromErrorCode) {
  ErrorMessage error_message{std::make_error_code(std::errc::io_error)};
  EXPECT_EQ(error_message.message(), std::make_error_code(std::errc::io_error).message());
}

TEST(Result, ErrorMessageCanBeEqualityCompared) {
  ErrorMessage error_message1{"Hello"};
  ErrorMessage error_message2{"Hello"};
  ErrorMessage error_message3{"Hello World!"};

  EXPECT_EQ(error_message1, error_message2);
  EXPECT_NE(error_message1, error_message3);
  EXPECT_NE(error_message2, error_message3);
}