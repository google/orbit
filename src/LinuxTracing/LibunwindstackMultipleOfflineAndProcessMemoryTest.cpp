// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "LibunwindstackMultipleOfflineAndProcessMemory.h"
#include "OrbitBase/ThreadUtils.h"
#include "unwindstack/Memory.h"

namespace orbit_linux_tracing {

TEST(LibunwindstackMultipleOfflineAndProcessMemory, ReadFromOneStackSlice) {
  constexpr uint64_t kStartAddress = 0xADD8E55;
  std::vector<uint8_t> bytes{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice(kStartAddress, bytes.size(), bytes.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory({stack_slice});

  std::array<uint8_t, 3> destination{};
  size_t read_count = sut->Read(kStartAddress + 2, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x20);
  EXPECT_EQ(destination[1], 0x30);
  EXPECT_EQ(destination[2], 0x40);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory, ReadFromFirstMatchingStackSlice) {
  constexpr uint64_t kStartAddress1 = 0xADD8E55;
  std::vector<uint8_t> bytes1{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice1(kStartAddress1, bytes1.size(), bytes1.data());

  constexpr uint64_t kStartAddress2 = 0xABCDEF;
  std::vector<uint8_t> bytes2{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
  StackSliceView stack_slice2(kStartAddress2, bytes2.size(), bytes2.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory(
          {stack_slice1, stack_slice2});

  std::array<uint8_t, 3> destination{};
  size_t read_count = sut->Read(kStartAddress1 + 2, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x20);
  EXPECT_EQ(destination[1], 0x30);
  EXPECT_EQ(destination[2], 0x40);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory, ReadFromSecondStackSlice) {
  constexpr uint64_t kStartAddress1 = 0xADD8E55;
  std::vector<uint8_t> bytes1{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice1(kStartAddress1, bytes1.size(), bytes1.data());

  constexpr uint64_t kStartAddress2 = 0xABCDEF;
  std::vector<uint8_t> bytes2{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
  StackSliceView stack_slice2(kStartAddress2, bytes2.size(), bytes2.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory(
          {stack_slice1, stack_slice2});

  std::array<uint8_t, 3> destination{};
  size_t read_count = sut->Read(kStartAddress2 + 2, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x13);
  EXPECT_EQ(destination[1], 0x14);
  EXPECT_EQ(destination[2], 0x15);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory,
     RequestingToReadWithPartialIntersectionReturnsZero) {
  constexpr uint64_t kStartAddress = 0xADD8E55;
  std::vector<uint8_t> bytes{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice(kStartAddress, bytes.size(), bytes.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory({stack_slice});

  std::array<uint8_t, 3> destination{};
  destination.fill(0x11);
  size_t read_count = sut->Read(kStartAddress - 1, destination.data(), 3);

  ASSERT_EQ(read_count, 0);
  EXPECT_EQ(destination[0], 0x11);
  EXPECT_EQ(destination[1], 0x11);
  EXPECT_EQ(destination[2], 0x11);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory,
     RequestingToReadUnknownMemoryWithoutProcessReturnsZero) {
  constexpr uint64_t kStartAddress = 0xADD8E55;
  std::vector<uint8_t> bytes{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice(kStartAddress, bytes.size(), bytes.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory({stack_slice});

  std::array<uint8_t, 3> destination{};
  destination.fill(0x11);
  size_t read_count = sut->Read(0xFE, destination.data(), 3);

  ASSERT_EQ(read_count, 0);
  EXPECT_EQ(destination[0], 0x11);
  EXPECT_EQ(destination[1], 0x11);
  EXPECT_EQ(destination[2], 0x11);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory,
     ReadFromCompleteMemoryEvenIfPartiallyIntersectsWithOtherStackSlice) {
  constexpr uint64_t kStartAddress1 = 0xADD8E55;
  std::vector<uint8_t> bytes1{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice1(kStartAddress1, bytes1.size(), bytes1.data());

  constexpr uint64_t kStartAddress2 = kStartAddress1 - 2;
  std::vector<uint8_t> bytes2{0x0a, 0x0b, 0x0c, 0x0d, 0x0f, 0x10, 0x01};
  StackSliceView stack_slice2(kStartAddress2, bytes2.size(), bytes2.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory(
          {stack_slice1, stack_slice2});

  std::array<uint8_t, 3> destination{};
  size_t read_count = sut->Read(kStartAddress2, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x0a);
  EXPECT_EQ(destination[1], 0x0b);
  EXPECT_EQ(destination[2], 0x0c);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory, ReadFromTestProcess) {
  constexpr uint64_t kStartAddress1 = 0xADD8E55;
  std::vector<uint8_t> bytes1{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice1(kStartAddress1, bytes1.size(), bytes1.data());

  constexpr uint64_t kStartAddress2 = 0xABCDEF;
  std::vector<uint8_t> bytes2{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
  StackSliceView stack_slice2(kStartAddress2, bytes2.size(), bytes2.data());

  std::vector<char> bytes3{0x09, 0x08, 0x07, 0x06, 0x05};

  uint32_t pid = orbit_base::GetCurrentProcessId();
  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithProcessMemory(
          pid, {stack_slice1, stack_slice2});

  std::array<char, 3> destination{};

  size_t read_count = sut->Read(absl::bit_cast<uint64_t>(bytes3.data()), destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x09);
  EXPECT_EQ(destination[1], 0x08);
  EXPECT_EQ(destination[2], 0x07);
}

}  // namespace orbit_linux_tracing
