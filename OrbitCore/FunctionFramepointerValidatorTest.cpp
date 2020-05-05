// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "FunctionFramepointerValidator.h"

#include <capstone/capstone.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

namespace {

constexpr uint8_t kFunctionWithFP[] = {0x55, 0x89, 0xE5, 0x83, 0xC0,
                                       0x01, 0xE8, 0x77, 0x00, 0x00,
                                       0x00, 0x89, 0xEC, 0x5D};
constexpr uint8_t kFunctionWithoutFP[] = {0x29, 0x25, 0x00, 0x00, 0x00, 0x00,
                                          0xE8, 0xFD, 0xFF, 0xFF, 0xFF, 0x01,
                                          0x25, 0x00, 0x00, 0x00, 0x00, 0xC3};
constexpr uint8_t kLeafFunction[] = {0x29, 0x24, 0x25, 0x00, 0x00, 0x00,
                                     0x00, 0x48, 0x01, 0x24, 0x25, 0x00,
                                     0x00, 0x00, 0x00, 0xC3};

TEST(FunctionFramepointerValidator, validateWithFP) {
  csh handle;
  size_t code_size = sizeof(kFunctionWithFP);
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramepointerValidator validator(handle, kFunctionWithFP, code_size);
  EXPECT_TRUE(validator.Validate());
  cs_close(&handle);
}

TEST(FunctionFramepointerValidator, validateWithoutFP) {
  csh handle;
  size_t code_size = sizeof(kFunctionWithoutFP);
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramepointerValidator validator(handle, kFunctionWithoutFP,
                                          code_size);
  EXPECT_FALSE(validator.Validate());
  cs_close(&handle);
}

TEST(FunctionFramepointerValidator, validateLeafFunction) {
  csh handle;
  size_t code_size = sizeof(kLeafFunction);
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramepointerValidator validator(handle, kLeafFunction, code_size);
  EXPECT_TRUE(validator.Validate());
  cs_close(&handle);
}

}  // namespace