// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <string>

#include "UserSpaceInstrumentation/AddressRange.h"
#include "UserSpaceInstrumentationAddressesImpl.h"

namespace orbit_linux_capture_service {

TEST(UserSpaceInstrumentationAddressesImpl, IsInEntryTrampolineWithNoAddressRanges) {
  UserSpaceInstrumentationAddressesImpl addresses{{}, {42, 84}, "/path/to/library.so"};

  EXPECT_FALSE(addresses.IsInEntryTrampoline(0));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(1));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(42));
}

TEST(UserSpaceInstrumentationAddressesImpl, IsInEntryTrampolineWithOneAddressRange) {
  UserSpaceInstrumentationAddressesImpl addresses{{{5, 10}}, {42, 84}, "/path/to/library.so"};

  EXPECT_FALSE(addresses.IsInEntryTrampoline(4));
  EXPECT_TRUE(addresses.IsInEntryTrampoline(5));
  EXPECT_TRUE(addresses.IsInEntryTrampoline(9));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(10));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(11));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(42));
}

TEST(UserSpaceInstrumentationAddressesImpl, IsInEntryTrampolineWithTwoAddressRanges) {
  UserSpaceInstrumentationAddressesImpl addresses{
      {{5, 10}, {15, 20}}, {42, 84}, "/path/to/library.so"};

  EXPECT_FALSE(addresses.IsInEntryTrampoline(4));
  EXPECT_TRUE(addresses.IsInEntryTrampoline(5));
  EXPECT_TRUE(addresses.IsInEntryTrampoline(9));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(10));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(11));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(14));
  EXPECT_TRUE(addresses.IsInEntryTrampoline(15));
  EXPECT_TRUE(addresses.IsInEntryTrampoline(19));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(20));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(21));
  EXPECT_FALSE(addresses.IsInEntryTrampoline(42));
}

TEST(UserSpaceInstrumentationAddressesImpl, IsInReturnTrampoline) {
  UserSpaceInstrumentationAddressesImpl addresses{
      {{5, 10}, {15, 20}}, {42, 84}, "/path/to/library.so"};

  EXPECT_FALSE(addresses.IsInReturnTrampoline(41));
  EXPECT_TRUE(addresses.IsInReturnTrampoline(42));
  EXPECT_TRUE(addresses.IsInReturnTrampoline(83));
  EXPECT_FALSE(addresses.IsInReturnTrampoline(84));
}

TEST(UserSpaceInstrumentationAddressesImpl, GetInjectedLibraryMapName) {
  UserSpaceInstrumentationAddressesImpl addresses{
      {{5, 10}, {15, 20}}, {42, 84}, "/path/to/library.so"};

  EXPECT_EQ(addresses.GetInjectedLibraryMapName(), "/path/to/library.so");
}

}  // namespace orbit_linux_capture_service