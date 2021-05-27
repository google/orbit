// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "AllocateInTracee.h"
#include "MachineCode.h"
#include "ObjectUtils/LinuxMap.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/TestUtils.h"
#include "RegisterState.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

namespace {

using absl::numbers_internal::safe_strtou64_base;
using orbit_base::HasError;
using orbit_base::HasNoError;
using orbit_base::HasValue;
using orbit_base::ReadFileToString;
using testing::ElementsAreArray;

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
  const std::vector<AddressRange> kUnavailableRanges1 = {
      {0, k64Kb}, {kOneGb, 2 * kOneGb}, {3 * kOneGb, 4 * kOneGb}};
  auto address_range_or_error =
      FindAddressRangeForTrampoline(kUnavailableRanges1, {kOneGb, 2 * kOneGb}, k256Mb);
  ASSERT_FALSE(address_range_or_error.has_error());
  EXPECT_EQ(kOneGb - k256Mb, address_range_or_error.value().start);

  // Placement to the left just fits.
  const std::vector<AddressRange> kUnavailableRanges2 = {
      {0, k64Kb}, {k256Mb, kOneGb}, {3 * kOneGb, 4 * kOneGb}};
  address_range_or_error =
      FindAddressRangeForTrampoline(kUnavailableRanges2, {k256Mb, kOneGb}, k256Mb - k64Kb);
  ASSERT_FALSE(address_range_or_error.has_error());
  EXPECT_EQ(k64Kb, address_range_or_error.value().start);

  // Placement to the left fails due to page alignment. So we place to the right which fits
  // trivially.
  const std::vector<AddressRange> kUnavailableRanges3 = {
      {0, k64Kb + 1}, {k256Mb, kOneGb}, {3 * kOneGb, 4 * kOneGb}};
  address_range_or_error =
      FindAddressRangeForTrampoline(kUnavailableRanges3, {k256Mb, kOneGb}, k256Mb - k64Kb - 5);
  ASSERT_FALSE(address_range_or_error.has_error());
  EXPECT_EQ(kOneGb, address_range_or_error.value().start);

  // Placement to the left just fits but only after a few hops.
  const std::vector<AddressRange> kUnavailableRanges4 = {
      {0, k64Kb},  // this is the gap that just fits
      {k64Kb + kOneMb, 6 * kOneMb},
      {6 * kOneMb + kOneMb - 1, 7 * kOneMb},
      {7 * kOneMb + kOneMb - 1, 8 * kOneMb},
      {8 * kOneMb + kOneMb - 1, 9 * kOneMb}};
  address_range_or_error = FindAddressRangeForTrampoline(
      kUnavailableRanges4, {8 * kOneMb + kOneMb - 1, 9 * kOneMb}, kOneMb);
  ASSERT_FALSE(address_range_or_error.has_error());
  EXPECT_EQ(k64Kb, address_range_or_error.value().start);

  // No space to the left but trivial placement to the right.
  const std::vector<AddressRange> kUnavailableRanges5 = {
      {0, k64Kb}, {kOneMb, kOneGb}, {5 * kOneGb, 6 * kOneGb}};
  address_range_or_error =
      FindAddressRangeForTrampoline(kUnavailableRanges5, {kOneMb, kOneGb}, kOneMb);
  ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
  EXPECT_EQ(kOneGb, address_range_or_error.value().start);

  // No space to the left but placement to the right works after a few hops.
  const std::vector<AddressRange> kUnavailableRanges6 = {
      {0, k64Kb},
      {kOneMb, kOneGb},
      {kOneGb + 0x01 * kOneMb - 1, kOneGb + 0x10 * kOneMb},
      {kOneGb + 0x11 * kOneMb - 1, kOneGb + 0x20 * kOneMb},
      {kOneGb + 0x21 * kOneMb - 1, kOneGb + 0x30 * kOneMb},
      {kOneGb + 0x31 * kOneMb - 1, kOneGb + 0x40 * kOneMb}};
  address_range_or_error =
      FindAddressRangeForTrampoline(kUnavailableRanges6, {kOneMb, kOneGb}, kOneMb);
  ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
  EXPECT_EQ(kOneGb + 0x40 * kOneMb, address_range_or_error.value().start);

  // No space to the left and the last segment nearly fills up the 64 bit address space. So no
  // placement is possible.
  const std::vector<AddressRange> kUnavailableRanges7 = {
      {0, k64Kb},
      {kOneMb, k256Mb},
      {1 * k256Mb + kOneMb - 1, 2 * k256Mb},
      {2 * k256Mb + kOneMb - 1, 3 * k256Mb},
      {3 * k256Mb + kOneMb - 1, 4 * k256Mb + 1},  // this gap is large but alignment doesn't fit
      {4 * k256Mb + kOneMb + 2, 5 * k256Mb},
      {5 * k256Mb + kOneMb - 1, 0xffffffffffffffff - kOneMb / 2}};
  address_range_or_error =
      FindAddressRangeForTrampoline(kUnavailableRanges7, {kOneMb, k256Mb}, kOneMb);
  ASSERT_TRUE(address_range_or_error.has_error());

  // There is no sufficiently large gap in the mappings in the 2GB below the code segment. So the
  // trampoline is placed above the code segment. Also we test that the trampoline starts at the
  // next memory page above last taken segment.
  const std::vector<AddressRange> kUnavailableRanges8 = {
      {0, k64Kb},  // huge gap here, but it's too far away
      {0x10 * kOneGb, 0x11 * kOneGb},
      {0x11 * kOneGb + kOneMb - 1, 0x12 * kOneGb},
      {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb + 42}};
  address_range_or_error = FindAddressRangeForTrampoline(
      kUnavailableRanges8, {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb}, kOneMb);
  ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
  constexpr uint64_t kPageSize = 4096;
  constexpr uint64_t kNextPage =
      (((0x12 * kOneGb + 2 * kOneMb + 42) + (kPageSize - 1)) / kPageSize) * kPageSize;
  EXPECT_EQ(kNextPage, address_range_or_error.value().start);

  // There is no sufficiently large gap in the mappings in the 2GB below the code segment. And there
  // also is no gap large enough in the 2GB above the code segment. So no placement is possible.
  const std::vector<AddressRange> kUnavailableRanges9 = {
      {0, k64Kb},  // huge gap here, but it's too far away
      {0x10 * kOneGb + kOneMb - 1, 0x11 * kOneGb},
      {0x11 * kOneGb + kOneMb - 1, 0x12 * kOneGb},
      {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb},
      {0x12 * kOneGb + 3 * kOneMb - 1, 0x13 * kOneGb + 1},
      {0x13 * kOneGb + kOneMb + 42, 0x14 * kOneGb}};
  address_range_or_error = FindAddressRangeForTrampoline(
      kUnavailableRanges9, {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb}, kOneMb);
  ASSERT_TRUE(address_range_or_error.has_error());

  // Fail on malformed into: first address range does not start at zero.
  const std::vector<AddressRange> kUnavailableRanges10 = {{k64Kb, kOneGb}};
  EXPECT_DEATH(
      auto result = FindAddressRangeForTrampoline(kUnavailableRanges10, {k64Kb, kOneGb}, kOneMb),
      "needs to start at zero");
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
  auto modules = orbit_object_utils::ReadModules(pid);
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

TEST(TrampolineTest, AddressDifferenceAsInt32) {
  // Result of the difference is negative; in the first case it just fits the second case overflows.
  const uint64_t kAddr1 = 0x6012345612345678;
  const uint64_t kAddr2Larger = kAddr1 - std::numeric_limits<int32_t>::min();
  auto result = AddressDifferenceAsInt32(kAddr1, kAddr2Larger);
  ASSERT_THAT(result, HasNoError());
  EXPECT_EQ(std::numeric_limits<int32_t>::min(), result.value());
  result = AddressDifferenceAsInt32(kAddr1, kAddr2Larger + 1);
  EXPECT_THAT(result, HasError("Difference is larger than +-2GB"));

  // Result of the difference is positive; in the first case it just fits the second case overflows.
  const uint64_t kAddr2Smaller = kAddr1 - std::numeric_limits<int32_t>::max();
  result = AddressDifferenceAsInt32(kAddr1, kAddr2Smaller);
  ASSERT_THAT(result, HasNoError());
  EXPECT_EQ(std::numeric_limits<int32_t>::max(), result.value());
  result = AddressDifferenceAsInt32(kAddr1, kAddr2Smaller - 1);
  EXPECT_THAT(result, HasError("Difference is larger than +-2GB"));

  // Result of the difference does not even fit into a int64. We handle that gracefully as well.
  const uint64_t kAddrHigh = 0xf234567812345678;
  const uint64_t kAddrLow = kAddrHigh - 0xe234567812345678;
  result = AddressDifferenceAsInt32(kAddrHigh, kAddrLow);
  EXPECT_THAT(result, HasError("Difference is larger than +-2GB"));
  result = AddressDifferenceAsInt32(kAddrLow, kAddrHigh);
  EXPECT_THAT(result, HasError("Difference is larger than +-2GB"));
}

class RelocateInstructionTest : public testing::Test {
 protected:
  void SetUp() override {
    CHECK(cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle_) == CS_ERR_OK);
    CHECK(cs_option(capstone_handle_, CS_OPT_DETAIL, CS_OPT_ON) == CS_ERR_OK);
    instruction_ = cs_malloc(capstone_handle_);
    CHECK(instruction_ != nullptr);
  }

  void Disassemble(const MachineCode& code) {
    const uint8_t* code_pointer = code.GetResultAsVector().data();
    size_t code_size = code.GetResultAsVector().size();
    uint64_t disassemble_address = 0;
    CHECK(cs_disasm_iter(capstone_handle_, &code_pointer, &code_size, &disassemble_address,
                         instruction_));
  }

  void TearDown() override {
    cs_free(instruction_, 1);
    cs_close(&capstone_handle_);
  }

  cs_insn* instruction_ = nullptr;

 private:
  csh capstone_handle_ = 0;
};

TEST_F(RelocateInstructionTest, TrivialTranslation) {
  MachineCode code;
  // nop
  code.AppendBytes({0x90});
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_THAT(result, HasValue());
  EXPECT_THAT(result.value().code, ElementsAreArray({0x90}));
  EXPECT_FALSE(result.value().position_of_absolute_address.has_value());
}

TEST_F(RelocateInstructionTest, RipRelativeAddressing) {
  MachineCode code;
  constexpr int32_t kOffset = 0x969433;
  // add qword ptr [rip + kOffset], 1
  code.AppendBytes({0x48, 0x83, 0x05}).AppendImmediate32(kOffset).AppendBytes({0x01});
  Disassemble(code);

  uint64_t kOriginalAddress = 0x0100000000;
  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress + kOffset - 0x123456);
  ASSERT_THAT(result, HasValue());
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x48, 0x83, 0x05, 0x56, 0x34, 0x12, 0x00, 0x01}));
  EXPECT_FALSE(result.value().position_of_absolute_address.has_value());

  result =
      RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress + kOffset + 0x123456);
  ASSERT_THAT(result, HasValue());
  // -0x123456 == 0xffedcbaa
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x48, 0x83, 0x05, 0xaa, 0xcb, 0xed, 0xff, 0x01}));
  EXPECT_FALSE(result.value().position_of_absolute_address.has_value());

  result = RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress - 0x7fff0000);
  EXPECT_THAT(result,
              HasError("While trying to relocate an instruction with rip relative addressing the "
                       "target was out of range from the trampoline."));
}

TEST_F(RelocateInstructionTest, UnconditionalJumpTo8BitImmediate) {
  MachineCode code;
  constexpr int8_t kOffset = 0x08;
  // jmp [rip + kOffset]
  code.AppendBytes({0xeb}).AppendImmediate8(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_THAT(result, HasValue());
  // jmp  [rip + 0]               ff 25 00 00 00 00
  // absolute_address             0a 00 00 00 01 00 00 00
  // original jump instruction ends on 0x0100000000 + 0x02. Adding kOffset (=8) yields 0x010000000a.
  EXPECT_THAT(result.value().code, ElementsAreArray({0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
                                                     0x00, 0x00, 0x01, 0x00, 0x00, 0x00}));
  ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(6, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, UnconditionalJumpTo32BitImmediate) {
  MachineCode code;
  constexpr int32_t kOffset = 0x01020304;
  // jmp [rip + kOffset]
  code.AppendBytes({0xe9}).AppendImmediate32(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_THAT(result, HasValue());
  // jmp  [rip + 0]               ff 25 00 00 00 00
  // absolute_address             09 03 02 01 01 00 00 00
  // original jump instruction end on 0x0100000000 + 0x05. Adding kOffset yields 0x0101020309.
  EXPECT_THAT(result.value().code, ElementsAreArray({0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x09, 0x03,
                                                     0x02, 0x01, 0x01, 0x00, 0x00, 0x00}));
  ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(6, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, CallToImmediateAddress) {
  MachineCode code;
  constexpr int32_t kOffset = 0x01020304;
  // call [rip + kOffset]
  code.AppendBytes({0xe8}).AppendImmediate32(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_THAT(result, HasValue());
  // Call [rip + 2]               ff 15 02 00 00 00
  // jmp  [rip + 8]               eb 08
  // absolute_address             09 03 02 01 01 00 00 00
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0xff, 0x15, 0x02, 0x00, 0x00, 0x00, 0xeb, 0x08, 0x09, 0x03, 0x02,
                                0x01, 0x01, 0x00, 0x00, 0x00}));
  ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(8, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, ConditionalJumpTo8BitImmediate) {
  MachineCode code;
  constexpr int8_t kOffset = 0x40;
  // jno rip + kOffset
  code.AppendBytes({0x71}).AppendImmediate8(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_THAT(result, HasValue());
  // jo rip + 16                  70 0e
  // jmp [rip + 6]                ff 25 00 00 00 00
  // absolute_address             42 00 00 00 01 00 00 00
  // original jump instruction ends on 0x0100000002 + 0x40 (kOffset) == 0x0100000042.
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x70, 0x0e, 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00,
                                0x00, 0x01, 0x00, 0x00, 0x00}));
  ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(8, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, ConditionalJumpTo32BitImmediate) {
  MachineCode code;
  constexpr int32_t kOffset = 0x12345678;
  // jno rip + kOffset           0f 80 78 56 34 12
  code.AppendBytes({0x0f, 0x80}).AppendImmediate32(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_TRUE(result.has_value());
  // jo rip + 16                  71 0e
  // jmp [rip +6]                 ff 25 00 00 00 00
  // absolute_address             7a 56 34 12 01 00 00 00
  // original jump instruction ends on 0x0100000006 + 0x12345678 (kOffset) == 0x011234567e.
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x71, 0x0e, 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x56, 0x34,
                                0x12, 0x01, 0x00, 0x00, 0x00}));
  ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(8, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, LoopIsUnsupported) {
  MachineCode code;
  constexpr int8_t kOffset = 0x40;
  // loopz rip + kOffset
  code.AppendBytes({0xe1}).AppendImmediate8(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  EXPECT_THAT(result, HasError("Relocating a loop instruction is not supported."));
}

}  // namespace orbit_user_space_instrumentation
