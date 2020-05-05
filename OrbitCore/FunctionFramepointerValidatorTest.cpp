#include "FunctionFramepointerValidator.h"
// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <capstone/capstone.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#define WITH_FP "\x55\x89\xE5\x83\xC0\x01\xE8\x77\x00\x00\x00\x89\xEC\x5D"
#define NO_FP \
  "\x29\x25\x00\x00\x00\x00\xE8\xFD\xFF\xFF\xFF\x01\x25\x00\x00\x00\x00\xC3"
#define LEAF "\x29\x24\x25\x00\x00\x00\x00\x48\x01\x24\x25\x00\x00\x00\x00\xC3"

namespace {

TEST(FunctionFramepointerValidator, validateWithFP) {
  csh handle;
  auto code = reinterpret_cast<const uint8_t*>(WITH_FP);
  size_t code_size = sizeof(WITH_FP) - 1;
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramepointerValidator validator(handle, code, code_size);
  EXPECT_TRUE(validator.validate());
  cs_close(&handle);
}

TEST(FunctionFramepointerValidator, validateWithoutFP) {
  csh handle;
  auto code = reinterpret_cast<const uint8_t*>(NO_FP);
  size_t code_size = sizeof(NO_FP) - 1;
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramepointerValidator validator(handle, code, code_size);
  EXPECT_FALSE(validator.validate());
  cs_close(&handle);
}

TEST(FunctionFramepointerValidator, validateLeafFunction) {
  csh handle;
  auto code = reinterpret_cast<const uint8_t*>(LEAF);
  size_t code_size = sizeof(LEAF) - 1;
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramepointerValidator validator(handle, code, code_size);
  EXPECT_TRUE(validator.validate());
  cs_close(&handle);
}

}  // namespace