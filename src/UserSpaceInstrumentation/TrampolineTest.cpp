// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "AllocateInTracee.h"
#include "Attach.h"
#include "ElfUtils/LinuxMap.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "Trampoline.h"

namespace orbit_user_space_instrumentation {

namespace {

using absl::numbers_internal::safe_strtou64_base;
using orbit_base::ReadFileToString;

extern "C" __attribute__((noinline)) int DoubleAndIncrement(int i) {
  i = 2 * i;
  return i + 1;
}

}  // namespace

TEST(TrampolineTest, DoAddressRangesOverlap) {
  AddressRange a = {3, 7};
  AddressRange b1 = {1, 2};
  EXPECT_FALSE(DoAddressRangesOverlap(a, b1));
  AddressRange b2 = {1, 3};
  EXPECT_FALSE(DoAddressRangesOverlap(a, b2));
  AddressRange b3 = {1, 4};
  EXPECT_TRUE(DoAddressRangesOverlap(a, b3));
  AddressRange b4 = {1, 9};
  EXPECT_TRUE(DoAddressRangesOverlap(a, b4));
  AddressRange b5 = {4, 5};
  EXPECT_TRUE(DoAddressRangesOverlap(a, b5));
  AddressRange b6 = {4, 9};
  EXPECT_TRUE(DoAddressRangesOverlap(a, b6));
  AddressRange b7 = {7, 9};
  EXPECT_FALSE(DoAddressRangesOverlap(a, b7));
  AddressRange b8 = {8, 9};
  EXPECT_FALSE(DoAddressRangesOverlap(a, b8));
}

TEST(TrampolineTest, LowestIntersectingAddressRange) {
  const std::vector<AddressRange> kAllRanges = {{0, 5}, {20, 30}, {40, 60}};

  EXPECT_FALSE(LowestIntersectingAddressRange({}, {0, 60}).has_value());

  EXPECT_EQ(0, LowestIntersectingAddressRange(kAllRanges, {1, 2}));
  EXPECT_EQ(1, LowestIntersectingAddressRange(kAllRanges, {21, 22}));
  EXPECT_EQ(2, LowestIntersectingAddressRange(kAllRanges, {51, 52}));

  EXPECT_EQ(0, LowestIntersectingAddressRange(kAllRanges, {3, 6}));
  EXPECT_EQ(1, LowestIntersectingAddressRange(kAllRanges, {19, 22}));
  EXPECT_EQ(2, LowestIntersectingAddressRange(kAllRanges, {30, 52}));

  EXPECT_EQ(0, LowestIntersectingAddressRange(kAllRanges, {4, 72}));
  EXPECT_EQ(1, LowestIntersectingAddressRange(kAllRanges, {29, 52}));
  EXPECT_EQ(2, LowestIntersectingAddressRange(kAllRanges, {59, 72}));

  EXPECT_FALSE(LowestIntersectingAddressRange(kAllRanges, {5, 20}).has_value());
  EXPECT_FALSE(LowestIntersectingAddressRange(kAllRanges, {30, 40}).has_value());
  EXPECT_FALSE(LowestIntersectingAddressRange(kAllRanges, {60, 80}).has_value());
}

TEST(TrampolineTest, HighestIntersectingAddressRange) {
  std::vector<AddressRange> kAllRanges = {{0, 5}, {20, 30}, {40, 60}};

  EXPECT_FALSE(HighestIntersectingAddressRange({}, {0, 60}).has_value());

  EXPECT_EQ(0, HighestIntersectingAddressRange(kAllRanges, {1, 2}));
  EXPECT_EQ(1, HighestIntersectingAddressRange(kAllRanges, {21, 22}));
  EXPECT_EQ(2, HighestIntersectingAddressRange(kAllRanges, {51, 52}));

  EXPECT_EQ(0, HighestIntersectingAddressRange(kAllRanges, {3, 6}));
  EXPECT_EQ(1, HighestIntersectingAddressRange(kAllRanges, {19, 22}));
  EXPECT_EQ(2, HighestIntersectingAddressRange(kAllRanges, {30, 52}));

  EXPECT_EQ(2, HighestIntersectingAddressRange(kAllRanges, {4, 72}));
  EXPECT_EQ(2, HighestIntersectingAddressRange(kAllRanges, {29, 52}));
  EXPECT_EQ(2, HighestIntersectingAddressRange(kAllRanges, {59, 72}));

  EXPECT_FALSE(HighestIntersectingAddressRange(kAllRanges, {5, 20}).has_value());
  EXPECT_FALSE(HighestIntersectingAddressRange(kAllRanges, {30, 40}).has_value());
  EXPECT_FALSE(HighestIntersectingAddressRange(kAllRanges, {60, 80}).has_value());
}

TEST(TrampolineTest, FindAddressRangeForTrampoline) {
  constexpr uint64_t k64Kb = 0x10000;
  constexpr uint64_t kOneMb = 0x100000;
  constexpr uint64_t k256Mb = 0x10000000;
  constexpr uint64_t kOneGb = 0x40000000;

  // Trivial placement to the left.
  const std::vector<AddressRange> kTakenRanges1 = {
      {0, k64Kb}, {kOneGb, 2 * kOneGb}, {3 * kOneGb, 4 * kOneGb}};
  auto address_range_or_error =
      FindAddressRangeForTrampoline(kTakenRanges1, {kOneGb, 2 * kOneGb}, k256Mb);
  ASSERT_FALSE(address_range_or_error.has_error());
  EXPECT_EQ(kOneGb - k256Mb, address_range_or_error.value().start);

  // Placement to the left just fits.
  const std::vector<AddressRange> kTakenRanges2 = {
      {0, k64Kb}, {k256Mb, kOneGb}, {3 * kOneGb, 4 * kOneGb}};
  address_range_or_error =
      FindAddressRangeForTrampoline(kTakenRanges2, {k256Mb, kOneGb}, k256Mb - k64Kb);
  ASSERT_FALSE(address_range_or_error.has_error());
  EXPECT_EQ(k64Kb, address_range_or_error.value().start);

  // Placement to the left fails due to page alignment. So we place to the right which fits
  // trivially.
  const std::vector<AddressRange> kTakenRanges3 = {
      {0, k64Kb + 1}, {k256Mb, kOneGb}, {3 * kOneGb, 4 * kOneGb}};
  address_range_or_error =
      FindAddressRangeForTrampoline(kTakenRanges3, {k256Mb, kOneGb}, k256Mb - k64Kb - 5);
  ASSERT_FALSE(address_range_or_error.has_error());
  EXPECT_EQ(kOneGb, address_range_or_error.value().start);

  // Placement to the left just fits but only after a few hops.
  const std::vector<AddressRange> kTakenRanges4 = {{0, k64Kb},  // this is the gap that just fits
                                                   {k64Kb + kOneMb, 6 * kOneMb},
                                                   {6 * kOneMb + kOneMb - 1, 7 * kOneMb},
                                                   {7 * kOneMb + kOneMb - 1, 8 * kOneMb},
                                                   {8 * kOneMb + kOneMb - 1, 9 * kOneMb}};
  address_range_or_error =
      FindAddressRangeForTrampoline(kTakenRanges4, {8 * kOneMb + kOneMb - 1, 9 * kOneMb}, kOneMb);
  ASSERT_FALSE(address_range_or_error.has_error());
  EXPECT_EQ(k64Kb, address_range_or_error.value().start);

  // No space to the left but trivial placement to the right.
  const std::vector<AddressRange> kTakenRanges5 = {
      {0, k64Kb}, {kOneMb, kOneGb}, {5 * kOneGb, 6 * kOneGb}};
  address_range_or_error = FindAddressRangeForTrampoline(kTakenRanges5, {kOneMb, kOneGb}, kOneMb);
  ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
  EXPECT_EQ(kOneGb, address_range_or_error.value().start);

  // No space to the left but placement to the right works after a few hops.
  const std::vector<AddressRange> kTakenRanges6 = {
      {0, k64Kb},
      {kOneMb, kOneGb},
      {kOneGb + 0x01 * kOneMb - 1, kOneGb + 0x10 * kOneMb},
      {kOneGb + 0x11 * kOneMb - 1, kOneGb + 0x20 * kOneMb},
      {kOneGb + 0x21 * kOneMb - 1, kOneGb + 0x30 * kOneMb},
      {kOneGb + 0x31 * kOneMb - 1, kOneGb + 0x40 * kOneMb}};
  address_range_or_error = FindAddressRangeForTrampoline(kTakenRanges6, {kOneMb, kOneGb}, kOneMb);
  ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
  EXPECT_EQ(kOneGb + 0x40 * kOneMb, address_range_or_error.value().start);

  // No space to the left and the last segment nearly fills up the 64 bit address space. So no
  // placement is possible.
  const std::vector<AddressRange> kTakenRanges7 = {
      {0, k64Kb},
      {kOneMb, k256Mb},
      {1 * k256Mb + kOneMb - 1, 2 * k256Mb},
      {2 * k256Mb + kOneMb - 1, 3 * k256Mb},
      {3 * k256Mb + kOneMb - 1, 4 * k256Mb + 1},  // this gap is large but alignment doesn't fit
      {4 * k256Mb + kOneMb + 2, 5 * k256Mb},
      {5 * k256Mb + kOneMb - 1, 0xffffffffffffffff - kOneMb / 2}};
  address_range_or_error = FindAddressRangeForTrampoline(kTakenRanges7, {kOneMb, k256Mb}, kOneMb);
  ASSERT_TRUE(address_range_or_error.has_error());

  // There is no sufficiently large gap in the mappings in the 2GB below the code segment. So the
  // trampoline is placed above the code segment. Also we test that the trampoline starts at the
  // next memory page above last taken segment.
  const std::vector<AddressRange> kTakenRanges8 = {
      {0, k64Kb},  // huge gap here, but it's too far away
      {0x10 * kOneGb, 0x11 * kOneGb},
      {0x11 * kOneGb + kOneMb - 1, 0x12 * kOneGb},
      {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb + 42}};
  address_range_or_error = FindAddressRangeForTrampoline(
      kTakenRanges8, {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb}, kOneMb);
  ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
  constexpr uint64_t kPageSize = 4096;
  constexpr uint64_t kNextPage =
      (((0x12 * kOneGb + 2 * kOneMb + 42) + (kPageSize - 1)) / kPageSize) * kPageSize;
  EXPECT_EQ(kNextPage, address_range_or_error.value().start);

  // There is no sufficiently large gap in the mappings in the 2GB below the code segment. And there
  // also is no gap large enough in the 2GB above the code segment. So no placement is possible.
  const std::vector<AddressRange> kTakenRanges9 = {
      {0, k64Kb},  // huge gap here, but it's too far away
      {0x10 * kOneGb + kOneMb - 1, 0x11 * kOneGb},
      {0x11 * kOneGb + kOneMb - 1, 0x12 * kOneGb},
      {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb},
      {0x12 * kOneGb + 3 * kOneMb - 1, 0x13 * kOneGb + 1},
      {0x13 * kOneGb + kOneMb + 42, 0x14 * kOneGb}};
  address_range_or_error = FindAddressRangeForTrampoline(
      kTakenRanges9, {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb}, kOneMb);
  ASSERT_TRUE(address_range_or_error.has_error());
}

TEST(TrampolineTest, AllocateMemoryForTrampolines) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    uint64_t sum = 0;
    int i = 0;
    while (true) {
      i = (i + 1) & 3;
      sum += DoubleAndIncrement(i);
    }
  }

  // Stop the process using our tooling.
  CHECK(AttachAndStopProcess(pid).has_value());

  // Find the address range of the code for `DoubleAndIncrement`. For the purpose of this test we
  // just take the entire address space taken up by `UserSpaceInstrumentationTests`.
  AddressRange code_range;
  auto modules = orbit_elf_utils::ReadModules(pid);
  CHECK(!modules.has_error());
  for (const auto& m : modules.value()) {
    if (m.name() == "UserSpaceInstrumentationTests") {
      code_range.start = m.address_start();
      code_range.end = m.address_end();
    }
  }

  // Allocate one megabyte in the tracee. The memory will be close to `code_range`.
  constexpr uint64_t kTrampolineSize = 1024 * 1024;
  auto address_or_error = AllocateMemoryForTrampolines(pid, code_range, kTrampolineSize);
  ASSERT_FALSE(address_or_error.has_error());

  // Check that the tracee is functional: Continue, stop again, free the allocated memory, then run
  // briefly again.
  CHECK(DetachAndContinueProcess(pid).has_value());
  CHECK(AttachAndStopProcess(pid).has_value());
  ASSERT_FALSE(FreeInTracee(pid, address_or_error.value(), kTrampolineSize).has_error());
  CHECK(DetachAndContinueProcess(pid).has_value());
  CHECK(AttachAndStopProcess(pid).has_value());

  // Detach and end child.
  CHECK(DetachAndContinueProcess(pid).has_value());
  kill(pid, SIGKILL);
  waitpid(pid, nullptr, 0);
}

}  // namespace orbit_user_space_instrumentation
