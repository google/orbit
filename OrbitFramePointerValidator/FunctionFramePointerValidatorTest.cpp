// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <capstone/capstone.h>
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "include/OrbitFramePointerValidator/FunctionFramePointerValidator.h"

namespace {

constexpr uint8_t kFunctionWithFP[] = {
    0x55,                          // push ebp
    0x89, 0xE5,                    // mov ebp,esp
    0x83, 0xC0, 0x01,              // add eax,0x1
    0xE8, 0x77, 0x00, 0x00, 0x00,  // call 0x82
    0x89, 0xEC,                    // mov esp,ebp
    0x5D,                          // pop ebp
    0xC3                           // ret
};
constexpr uint8_t kFunctionWithoutFP[] = {
    0x29, 0x25, 0x00, 0x00, 0x00, 0x00,  // sub DWORD PTR ds:0x0,esp
    0xE8, 0xFD, 0xFF, 0xFF, 0xFF,        // call 0x8
    0x01, 0x25, 0x00, 0x00, 0x00, 0x00,  // add DWORD PTR ds:0x0,esp
    0xC3                                 // ret
};
constexpr uint8_t kLeafFunction[] = {
    0x29, 0x24, 0x25, 0x00, 0x00, 0x00, 0x00,  // sub DWORD PTR [eiz*1+0x0],esp
    0x48,                                      // dec eax
    0x01, 0x24, 0x25, 0x00, 0x00, 0x00, 0x00,  // add DWORD PTR [eiz*1+0x0],esp
    0xC3                                       // ret
};
constexpr uint8_t kFunctionWithEnterLeaveWithFP[] = {
    0xC8, 0x00, 0x00, 0x01,        // enter 0x0,0x1
    0x83, 0xC0, 0x0A,              // add eax,0xa
    0xE8, 0xFC, 0xFF, 0xFF, 0xFF,  // call 0x8
    0xC9,                          // leave
    0xC3                           // ret
};
constexpr uint8_t kTailFunctionWithFP[] = {
    0x55,                          // push ebp
    0x89, 0xE5,                    // mov ebp,esp
    0x83, 0xC0, 0x0A,              // add eax,0xa
    0xE8, 0xFC, 0xFF, 0xFF, 0xFF,  // call 0x7
    0x89, 0xEC,                    // mov esp,ebp
    0x5D,                          // pop ebp
    0xFF, 0xE0                     // jmp eax
};

TEST(FunctionFramePointerValidator, validateWithFP) {
  csh handle;
  size_t code_size = sizeof(kFunctionWithFP);
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramePointerValidator validator(handle, kFunctionWithFP, code_size);
  EXPECT_TRUE(validator.Validate());
  cs_close(&handle);
}

TEST(FunctionFramePointerValidator, validateWithoutFP) {
  csh handle;
  size_t code_size = sizeof(kFunctionWithoutFP);
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramePointerValidator validator(handle, kFunctionWithoutFP,
                                          code_size);
  EXPECT_FALSE(validator.Validate());
  cs_close(&handle);
}

TEST(FunctionFramePointerValidator, validateLeafFunction) {
  csh handle;
  size_t code_size = sizeof(kLeafFunction);
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramePointerValidator validator(handle, kLeafFunction, code_size);
  EXPECT_TRUE(validator.Validate());
  cs_close(&handle);
}

TEST(FunctionFramePointerValidator, validateEnterLeave) {
  csh handle;
  size_t code_size = sizeof(kFunctionWithEnterLeaveWithFP);
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramePointerValidator validator(handle, kFunctionWithEnterLeaveWithFP,
                                          code_size);
  EXPECT_TRUE(validator.Validate());
  cs_close(&handle);
}

TEST(FunctionFramePointerValidator, validateTailFunction) {
  csh handle;
  size_t code_size = sizeof(kTailFunctionWithFP);
  ASSERT_EQ(cs_open(CS_ARCH_X86, CS_MODE_32, &handle), CS_ERR_OK);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  FunctionFramePointerValidator validator(handle, kTailFunctionWithFP,
                                          code_size);
  EXPECT_TRUE(validator.Validate());
  cs_close(&handle);
}

}  // namespace