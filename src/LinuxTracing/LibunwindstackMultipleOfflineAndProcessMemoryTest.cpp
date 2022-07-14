// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "LibunwindstackMultipleOfflineAndProcessMemory.h"

using ::testing::Invoke;

namespace orbit_linux_tracing {

namespace {
class MockProcessMemory : public unwindstack::Memory {
 public:
  MOCK_METHOD(size_t, Read, (uint64_t, void*, size_t), (override));
};
}  // namespace

TEST(LibunwindstackMultipleOfflineAndProcessMemory, ReadFromOneStackSlice) {
  constexpr uint64_t kStartAddress = 0xADD8E55;
  std::vector<char> bytes{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice(kStartAddress, bytes.size(), bytes.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory({stack_slice});

  std::array<char, 3> destination{};
  size_t read_count = sut->Read(kStartAddress + 2, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x20);
  EXPECT_EQ(destination[1], 0x30);
  EXPECT_EQ(destination[2], 0x40);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory, ReadFromFirstMatchingStackSlice) {
  constexpr uint64_t kStartAddress1 = 0xADD8E55;
  std::vector<char> bytes1{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice1(kStartAddress1, bytes1.size(), bytes1.data());

  constexpr uint64_t kStartAddress2 = 0xABCDEF;
  std::vector<char> bytes2{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
  StackSliceView stack_slice2(kStartAddress2, bytes2.size(), bytes2.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory(
          {stack_slice1, stack_slice2});

  std::array<char, 3> destination{};
  size_t read_count = sut->Read(kStartAddress1 + 2, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x20);
  EXPECT_EQ(destination[1], 0x30);
  EXPECT_EQ(destination[2], 0x40);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory, ReadFromSecondStackSlice) {
  constexpr uint64_t kStartAddress1 = 0xADD8E55;
  std::vector<char> bytes1{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice1(kStartAddress1, bytes1.size(), bytes1.data());

  constexpr uint64_t kStartAddress2 = 0xABCDEF;
  std::vector<char> bytes2{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
  StackSliceView stack_slice2(kStartAddress2, bytes2.size(), bytes2.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory(
          {stack_slice1, stack_slice2});

  std::array<char, 3> destination{};
  size_t read_count = sut->Read(kStartAddress2 + 2, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x13);
  EXPECT_EQ(destination[1], 0x14);
  EXPECT_EQ(destination[2], 0x15);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory,
     RequestingToReadWithPartialIntersectionReturnsZero) {
  constexpr uint64_t kStartAddress = 0xADD8E55;
  std::vector<char> bytes{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice(kStartAddress, bytes.size(), bytes.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory({stack_slice});

  std::array<char, 3> destination{};
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
  std::vector<char> bytes{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice(kStartAddress, bytes.size(), bytes.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory({stack_slice});

  std::array<char, 3> destination{};
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
  std::vector<char> bytes1{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice1(kStartAddress1, bytes1.size(), bytes1.data());

  constexpr uint64_t kStartAddress2 = kStartAddress1 + 2;
  std::vector<char> bytes2{0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x77};
  StackSliceView stack_slice2(kStartAddress2, bytes2.size(), bytes2.data());

  std::shared_ptr<unwindstack::Memory> sut =
      LibunwindstackMultipleOfflineAndProcessMemory::CreateWithoutProcessMemory(
          {stack_slice1, stack_slice2});

  std::array<char, 3> destination{};
  size_t read_count = sut->Read(kStartAddress2, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x20);
  EXPECT_EQ(destination[1], 0x30);
  EXPECT_EQ(destination[2], 0x40);
}

TEST(LibunwindstackMultipleOfflineAndProcessMemory, ReadFromProcess) {
  constexpr uint64_t kStartAddress1 = 0xADD8E55;
  std::vector<char> bytes1{0x01, 0x10, 0x20, 0x30, 0x40};
  StackSliceView stack_slice1(kStartAddress1, bytes1.size(), bytes1.data());

  constexpr uint64_t kStartAddress2 = 0xABCDEF;
  std::vector<char> bytes2{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
  StackSliceView stack_slice2(kStartAddress2, bytes2.size(), bytes2.data());

  std::vector<StackSliceView> stack_slices{stack_slice1, stack_slice2};
  std::vector<LibunwindstackOfflineMemory> stack_memory_slices{};
  stack_memory_slices.reserve(stack_slices.size());
  for (const StackSliceView& stack_slice_view : stack_slices) {
    stack_memory_slices.emplace_back(stack_slice_view);
  }

  std::shared_ptr<MockProcessMemory> process_memory_mock = std::make_shared<MockProcessMemory>();
  LibunwindstackMultipleOfflineAndProcessMemory sut{process_memory_mock,
                                                    std::move(stack_memory_slices)};

  std::array<char, 3> destination{};
  EXPECT_CALL(*process_memory_mock, Read(0xFE, destination.data(), 3))
      .Times(1)
      .WillOnce(Invoke([](uint64_t /*addr*/, void* dst, size_t /*size*/) -> size_t {
        auto destination = absl::bit_cast<char*>(dst);
        destination[0] = 0x11;
        destination[1] = 0x22;
        destination[2] = 0x33;
        return 3;
      }));

  size_t read_count = sut.Read(0xFE, destination.data(), 3);

  ASSERT_EQ(read_count, 3);
  EXPECT_EQ(destination[0], 0x11);
  EXPECT_EQ(destination[1], 0x22);
  EXPECT_EQ(destination[2], 0x33);
}

}  // namespace orbit_linux_tracing
