// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gtest/gtest.h>

#include "StackSlice.h"

namespace orbit_linux_tracing {

TEST(StackSlice, CanBeConstructed) {
  constexpr uint64_t kSize = 3;
  constexpr char kVal1 = 0x12;
  constexpr char kVal2 = 0x42;
  constexpr char kVal3 = 0x1F;
  std::unique_ptr<char[]> data{new char[kSize]{kVal1, kVal2, kVal3}};
  constexpr uint64_t kStartAddress = 42;

  StackSlice stack_slice{kStartAddress, kSize, std::move(data)};

  EXPECT_EQ(stack_slice.GetStartAddress(), kStartAddress);
  EXPECT_EQ(stack_slice.GetSize(), kSize);
  EXPECT_EQ(stack_slice.GetData()[0], kVal1);
  EXPECT_EQ(stack_slice.GetData()[1], kVal2);
  EXPECT_EQ(stack_slice.GetData()[2], kVal3);
}

TEST(StackSliceView, CanBeConstructedFromStackSlice) {
  constexpr uint64_t kSize = 3;
  constexpr char kVal1 = 0x12;
  constexpr char kVal2 = 0x42;
  constexpr char kVal3 = 0x1F;
  std::unique_ptr<char[]> data{new char[kSize]{kVal1, kVal2, kVal3}};
  constexpr uint64_t kStartAddress = 42;

  StackSlice stack_slice{kStartAddress, kSize, std::move(data)};

  StackSliceView stack_slice_view{stack_slice};

  EXPECT_EQ(stack_slice_view.GetStartAddress(), kStartAddress);
  EXPECT_EQ(stack_slice_view.GetSize(), kSize);
  EXPECT_EQ(stack_slice_view.GetEndAddress(), kStartAddress + kSize);
  EXPECT_EQ(stack_slice_view.GetData()[0], kVal1);
  EXPECT_EQ(stack_slice_view.GetData()[1], kVal2);
  EXPECT_EQ(stack_slice_view.GetData()[2], kVal3);
}

TEST(StackSliceView, CanBeConstructedFromRawData) {
  constexpr char kVal1 = 0x12;
  constexpr char kVal2 = 0x42;
  constexpr char kVal3 = 0x1F;
  std::vector<char> data{kVal1, kVal2, kVal3};
  constexpr uint64_t kStartAddress = 42;

  StackSliceView stack_slice_view{kStartAddress, data.size(), data.data()};

  EXPECT_EQ(stack_slice_view.GetStartAddress(), kStartAddress);
  EXPECT_EQ(stack_slice_view.GetSize(), data.size());
  EXPECT_EQ(stack_slice_view.GetEndAddress(), kStartAddress + data.size());
  EXPECT_EQ(stack_slice_view.GetData()[0], kVal1);
  EXPECT_EQ(stack_slice_view.GetData()[1], kVal2);
  EXPECT_EQ(stack_slice_view.GetData()[2], kVal3);
}

}  // namespace orbit_linux_tracing