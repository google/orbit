// // Copyright (c) 2021 The Orbit Authors. All rights reserved.
// // Use of this source code is governed by a BSD-style license that can be
// // found in the LICENSE file.

// #include <absl/base/casts.h>
// #include <absl/container/flat_hash_map.h>
// #include <absl/hash/hash.h>
// #include <capstone/capstone.h>
// #include <dlfcn.h>
// #include <gmock/gmock.h>
// #include <gtest/gtest.h>
// #include <immintrin.h>
// #include <signal.h>
// #include <sys/prctl.h>
// #include <sys/wait.h>
// #include <unistd.h>

// #include <algorithm>
// #include <chrono>
// #include <cstdint>
// #include <filesystem>
// #include <limits>
// #include <memory>
// #include <numeric>
// #include <optional>
// #include <random>
// #include <string>
// #include <string_view>
// #include <thread>
// #include <utility>
// #include <vector>

// #include "AccessTraceesMemory.h"
// #include "AllocateInTracee.h"
// #include "GetTestLibLibraryPath.h"
// #include "GrpcProtos/module.pb.h"
// #include "MachineCode.h"
// #include "ModuleUtils/ReadLinuxModules.h"
// #include "OrbitBase/ExecutablePath.h"
// #include "OrbitBase/Logging.h"
// #include "OrbitBase/Result.h"
// #include "TestUtils.h"
// #include "TestUtils/TestUtils.h"
// #include "Trampoline.h"
// #include "UserSpaceInstrumentation/AddressRange.h"
// #include "UserSpaceInstrumentation/Attach.h"
// #include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

// namespace orbit_user_space_instrumentation {
// using orbit_test_utils::HasErrorWithMessage;

// namespace {

// using orbit_test_utils::HasErrorWithMessage;
// using orbit_test_utils::HasNoError;
// using orbit_test_utils::HasValue;
// using testing::ElementsAreArray;

// constexpr const char* kEntryPayloadFunctionName = "EntryPayload";
// constexpr const char* kExitPayloadFunctionName = "ExitPayload";

// extern "C" __attribute__((noinline)) int DoubleAndIncrement(int i) {
//   i = 2 * i;
//   return i + 1;
// }

// }  // namespace

// TEST(TrampolineTest, DoAddressRangesOverlap) {
//   AddressRange a = {3, 7};
//   AddressRange b1 = {1, 2};
//   EXPECT_FALSE(DoAddressRangesOverlap(a, b1));
//   AddressRange b2 = {1, 3};
//   EXPECT_FALSE(DoAddressRangesOverlap(a, b2));
//   AddressRange b3 = {1, 4};
//   EXPECT_TRUE(DoAddressRangesOverlap(a, b3));
//   AddressRange b4 = {1, 9};
//   EXPECT_TRUE(DoAddressRangesOverlap(a, b4));
//   AddressRange b5 = {4, 5};
//   EXPECT_TRUE(DoAddressRangesOverlap(a, b5));
//   AddressRange b6 = {4, 9};
//   EXPECT_TRUE(DoAddressRangesOverlap(a, b6));
//   AddressRange b7 = {7, 9};
//   EXPECT_FALSE(DoAddressRangesOverlap(a, b7));
//   AddressRange b8 = {8, 9};
//   EXPECT_FALSE(DoAddressRangesOverlap(a, b8));
// }

// TEST(TrampolineTest, LowestIntersectingAddressRange) {
//   const std::vector<AddressRange> all_ranges = {{0, 5}, {20, 30}, {40, 60}};

//   EXPECT_FALSE(LowestIntersectingAddressRange({}, {0, 60}).has_value());

//   EXPECT_EQ(0, LowestIntersectingAddressRange(all_ranges, {1, 2}));
//   EXPECT_EQ(1, LowestIntersectingAddressRange(all_ranges, {21, 22}));
//   EXPECT_EQ(2, LowestIntersectingAddressRange(all_ranges, {51, 52}));

//   EXPECT_EQ(0, LowestIntersectingAddressRange(all_ranges, {3, 6}));
//   EXPECT_EQ(1, LowestIntersectingAddressRange(all_ranges, {19, 22}));
//   EXPECT_EQ(2, LowestIntersectingAddressRange(all_ranges, {30, 52}));

//   EXPECT_EQ(0, LowestIntersectingAddressRange(all_ranges, {4, 72}));
//   EXPECT_EQ(1, LowestIntersectingAddressRange(all_ranges, {29, 52}));
//   EXPECT_EQ(2, LowestIntersectingAddressRange(all_ranges, {59, 72}));

//   EXPECT_FALSE(LowestIntersectingAddressRange(all_ranges, {5, 20}).has_value());
//   EXPECT_FALSE(LowestIntersectingAddressRange(all_ranges, {30, 40}).has_value());
//   EXPECT_FALSE(LowestIntersectingAddressRange(all_ranges, {60, 80}).has_value());
// }

// TEST(TrampolineTest, HighestIntersectingAddressRange) {
//   const std::vector<AddressRange> all_ranges = {{0, 5}, {20, 30}, {40, 60}};

//   EXPECT_FALSE(HighestIntersectingAddressRange({}, {0, 60}).has_value());

//   EXPECT_EQ(0, HighestIntersectingAddressRange(all_ranges, {1, 2}));
//   EXPECT_EQ(1, HighestIntersectingAddressRange(all_ranges, {21, 22}));
//   EXPECT_EQ(2, HighestIntersectingAddressRange(all_ranges, {51, 52}));

//   EXPECT_EQ(0, HighestIntersectingAddressRange(all_ranges, {3, 6}));
//   EXPECT_EQ(1, HighestIntersectingAddressRange(all_ranges, {19, 22}));
//   EXPECT_EQ(2, HighestIntersectingAddressRange(all_ranges, {30, 52}));

//   EXPECT_EQ(2, HighestIntersectingAddressRange(all_ranges, {4, 72}));
//   EXPECT_EQ(2, HighestIntersectingAddressRange(all_ranges, {29, 52}));
//   EXPECT_EQ(2, HighestIntersectingAddressRange(all_ranges, {59, 72}));

//   EXPECT_FALSE(HighestIntersectingAddressRange(all_ranges, {5, 20}).has_value());
//   EXPECT_FALSE(HighestIntersectingAddressRange(all_ranges, {30, 40}).has_value());
//   EXPECT_FALSE(HighestIntersectingAddressRange(all_ranges, {60, 80}).has_value());
// }

// TEST(TrampolineTest, FindAddressRangeForTrampoline) {
//   constexpr uint64_t k1Kb = 0x400;
//   constexpr uint64_t k64Kb = 0x10000;
//   constexpr uint64_t kOneMb = 0x100000;
//   constexpr uint64_t k256Mb = 0x10000000;
//   constexpr uint64_t kOneGb = 0x40000000;

//   // Trivial placement to the left.
//   const std::vector<AddressRange> unavailable_ranges1 = {
//       {0, k64Kb}, {kOneGb, 2 * kOneGb}, {3 * kOneGb, 4 * kOneGb}};
//   auto address_range_or_error =
//       FindAddressRangeForTrampoline(unavailable_ranges1, {kOneGb, 2 * kOneGb}, k256Mb);
//   ASSERT_FALSE(address_range_or_error.has_error());
//   EXPECT_EQ(kOneGb - k256Mb, address_range_or_error.value().start);

//   // Placement to the left just fits.
//   const std::vector<AddressRange> unavailable_ranges2 = {
//       {0, k64Kb}, {k256Mb, kOneGb}, {3 * kOneGb, 4 * kOneGb}};
//   address_range_or_error =
//       FindAddressRangeForTrampoline(unavailable_ranges2, {k256Mb, kOneGb}, k256Mb - k64Kb);
//   ASSERT_FALSE(address_range_or_error.has_error());
//   EXPECT_EQ(k64Kb, address_range_or_error.value().start);

//   // Placement to the left fails due to page alignment. So we place to the right which fits
//   // trivially.
//   const std::vector<AddressRange> unavailable_ranges3 = {
//       {0, k64Kb + 1}, {k256Mb, kOneGb}, {3 * kOneGb, 4 * kOneGb}};
//   address_range_or_error =
//       FindAddressRangeForTrampoline(unavailable_ranges3, {k256Mb, kOneGb}, k256Mb - k64Kb - 5);
//   ASSERT_FALSE(address_range_or_error.has_error());
//   EXPECT_EQ(kOneGb, address_range_or_error.value().start);

//   // Placement to the left just fits but only after a few hops.
//   const std::vector<AddressRange> unavailable_ranges4 = {
//       {0, k64Kb},  // this is the gap that just fits
//       {k64Kb + kOneMb, 6 * kOneMb},
//       {6 * kOneMb + kOneMb - 1, 7 * kOneMb},
//       {7 * kOneMb + kOneMb - 1, 8 * kOneMb},
//       {8 * kOneMb + kOneMb - 1, 9 * kOneMb}};
//   address_range_or_error = FindAddressRangeForTrampoline(
//       unavailable_ranges4, {8 * kOneMb + kOneMb - 1, 9 * kOneMb}, kOneMb);
//   ASSERT_FALSE(address_range_or_error.has_error());
//   EXPECT_EQ(k64Kb, address_range_or_error.value().start);

//   // No space to the left but trivial placement to the right.
//   const std::vector<AddressRange> unavailable_ranges5 = {
//       {0, k64Kb}, {kOneMb, kOneGb}, {5 * kOneGb, 6 * kOneGb}};
//   address_range_or_error =
//       FindAddressRangeForTrampoline(unavailable_ranges5, {kOneMb, kOneGb}, kOneMb);
//   ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
//   EXPECT_EQ(kOneGb, address_range_or_error.value().start);

//   // No space to the left but placement to the right works after a few hops.
//   const std::vector<AddressRange> unavailable_ranges6 = {
//       {0, k64Kb},
//       {kOneMb, kOneGb},
//       {kOneGb + 0x01 * kOneMb - 1, kOneGb + 0x10 * kOneMb},
//       {kOneGb + 0x11 * kOneMb - 1, kOneGb + 0x20 * kOneMb},
//       {kOneGb + 0x21 * kOneMb - 1, kOneGb + 0x30 * kOneMb},
//       {kOneGb + 0x31 * kOneMb - 1, kOneGb + 0x40 * kOneMb}};
//   address_range_or_error =
//       FindAddressRangeForTrampoline(unavailable_ranges6, {kOneMb, kOneGb}, kOneMb);
//   ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
//   EXPECT_EQ(kOneGb + 0x40 * kOneMb, address_range_or_error.value().start);

//   // No space to the left and the last segment nearly fills up the 64 bit address space. So no
//   // placement is possible.
//   const std::vector<AddressRange> unavailable_ranges7 = {
//       {0, k64Kb},
//       {kOneMb, k256Mb},
//       {1 * k256Mb + kOneMb - 1, 2 * k256Mb},
//       {2 * k256Mb + kOneMb - 1, 3 * k256Mb},
//       {3 * k256Mb + kOneMb - 1, 4 * k256Mb + 1},  // this gap is large but alignment doesn't fit
//       {4 * k256Mb + kOneMb + 2, 5 * k256Mb},
//       {5 * k256Mb + kOneMb - 1, 0xffffffffffffffff - kOneMb / 2}};
//   address_range_or_error =
//       FindAddressRangeForTrampoline(unavailable_ranges7, {kOneMb, k256Mb}, kOneMb);
//   ASSERT_TRUE(address_range_or_error.has_error());

//   // There is no sufficiently large gap in the mappings in the 2GB below the code segment. So the
//   // trampoline is placed above the code segment. Also we test that the trampoline starts at the
//   // next memory page above last taken segment.
//   const std::vector<AddressRange> unavailable_ranges8 = {
//       {0, k64Kb},  // huge gap here, but it's too far away
//       {0x10 * kOneGb, 0x11 * kOneGb},
//       {0x11 * kOneGb + kOneMb - 1, 0x12 * kOneGb},
//       {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb + 42}};
//   address_range_or_error = FindAddressRangeForTrampoline(
//       unavailable_ranges8, {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb}, kOneMb);
//   ASSERT_FALSE(address_range_or_error.has_error()) << address_range_or_error.error().message();
//   constexpr uint64_t kPageSize = 4096;
//   constexpr uint64_t kNextPage =
//       (((0x12 * kOneGb + 2 * kOneMb + 42) + (kPageSize - 1)) / kPageSize) * kPageSize;
//   EXPECT_EQ(kNextPage, address_range_or_error.value().start);

//   // There is no sufficiently large gap in the mappings in the 2GB below the code segment. And there
//   // also is no gap large enough in the 2GB above the code segment. So no placement is possible.
//   const std::vector<AddressRange> unavailable_ranges9 = {
//       {0, k64Kb},  // huge gap here, but it's too far away
//       {0x10 * kOneGb + kOneMb - 1, 0x11 * kOneGb},
//       {0x11 * kOneGb + kOneMb - 1, 0x12 * kOneGb},
//       {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb},
//       {0x12 * kOneGb + 3 * kOneMb - 1, 0x13 * kOneGb + 1},
//       {0x13 * kOneGb + kOneMb + 42, 0x14 * kOneGb}};
//   address_range_or_error = FindAddressRangeForTrampoline(
//       unavailable_ranges9, {0x12 * kOneGb + kOneMb - 1, 0x12 * kOneGb + 2 * kOneMb}, kOneMb);
//   ASSERT_TRUE(address_range_or_error.has_error());

//   // Fail on malformed input: first address range does not start at zero.
//   const std::vector<AddressRange> unavailable_ranges10 = {{k64Kb, kOneGb}};
//   EXPECT_DEATH(
//       auto result = FindAddressRangeForTrampoline(unavailable_ranges10, {k64Kb, kOneGb}, kOneMb),
//       "needs to start at zero");

//   // Placement to the left fails since the requested memory chunk is too big. So we place to the
//   // right which fits trivially.
//   // The special case here is that the requested memory size (k256Mb + k64Kb) is larger than the
//   // left interval border of the second interval (k256Mb). This produced an artithmetic overflow in
//   // a previous version of the algorithm.
//   const std::vector<AddressRange> unavailable_ranges11 = {{0, k64Kb}, {k256Mb, kOneGb}};
//   address_range_or_error =
//       FindAddressRangeForTrampoline(unavailable_ranges11, {k256Mb, kOneGb}, k256Mb + k64Kb);
//   ASSERT_FALSE(address_range_or_error.has_error());
//   EXPECT_EQ(kOneGb, address_range_or_error.value().start);

//   // Placement to the left fails, placement to the right fails also because we are close to the end
//   // of the address space. This produced an artithmetic overflow in a previous version of the
//   // algorithm.
//   const std::vector<AddressRange> unavailable_ranges12 = {
//       {0, k64Kb},
//       {UINT64_MAX - 10 * kOneGb, UINT64_MAX - k64Kb - 1},
//       {UINT64_MAX - k64Kb, UINT64_MAX - k1Kb}};
//   address_range_or_error = FindAddressRangeForTrampoline(
//       unavailable_ranges12, {UINT64_MAX - k64Kb, UINT64_MAX - k1Kb}, k64Kb);
//   ASSERT_THAT(address_range_or_error, HasErrorWithMessage("No place to fit"));

//   // We can not fit anything close to a range larger than 2GB.
//   const std::vector<AddressRange> unavailable_ranges13 = {{0, k64Kb}, {kOneGb, 4 * kOneGb}};
//   address_range_or_error =
//       FindAddressRangeForTrampoline(unavailable_ranges13, {kOneGb, 4 * kOneGb}, k64Kb);
//   ASSERT_THAT(address_range_or_error, HasErrorWithMessage("No place to fit"));
// }

// TEST(TrampolineTest, AllocateMemoryForTrampolines) {
//   pid_t pid = fork();
//   ORBIT_CHECK(pid != -1);
//   if (pid == 0) {
//     prctl(PR_SET_PDEATHSIG, SIGTERM);

//     [[maybe_unused]] volatile uint64_t sum = 0;
//     // Endless loops without side effects are UB and recent versions of clang optimize
//     // it away. Making `i` volatile avoids that problem.
//     volatile int i = 0;
//     while (true) {
//       i = (i + 1) & 3;
//       sum += DoubleAndIncrement(i);
//     }
//   }

//   // Stop the process using our tooling.
//   ORBIT_CHECK(AttachAndStopProcess(pid).has_value());

//   // Find the address range of the code for `DoubleAndIncrement`. For the purpose of this test we
//   // just take the entire address space taken up by `UserSpaceInstrumentationTests`.
//   auto modules_or_error = orbit_module_utils::ReadModules(pid);
//   ORBIT_CHECK(!modules_or_error.has_error());

//   auto& modules = modules_or_error.value();
//   const auto module = std::find_if(modules.begin(), modules.end(), [&](const auto& module) {
//     return module.file_path() == orbit_base::GetExecutablePath();
//   });

//   ASSERT_NE(module, modules.end());
//   const AddressRange code_range{module->address_start(), module->address_end()};

//   // Allocate one megabyte in the tracee. The memory will be close to `code_range`.
//   constexpr uint64_t kTrampolineSize = 1024 * 1024;
//   auto memory_or_error = AllocateMemoryForTrampolines(pid, code_range, kTrampolineSize);
//   ASSERT_FALSE(memory_or_error.has_error());

//   // Check that the tracee is functional: Continue, stop again, free the allocated memory, then run
//   // briefly again.
//   ORBIT_CHECK(DetachAndContinueProcess(pid).has_value());
//   ORBIT_CHECK(AttachAndStopProcess(pid).has_value());
//   ASSERT_THAT(memory_or_error.value()->Free(), HasNoError());
//   ORBIT_CHECK(DetachAndContinueProcess(pid).has_value());
//   ORBIT_CHECK(AttachAndStopProcess(pid).has_value());

//   // Detach and end child.
//   ORBIT_CHECK(DetachAndContinueProcess(pid).has_value());
//   kill(pid, SIGKILL);
//   waitpid(pid, nullptr, 0);
// }

// TEST(TrampolineTest, AddressDifferenceAsInt32) {
//   // Result of the difference is negative; in the first case it just fits, the second case
//   // overflows.
//   constexpr uint64_t kAddr1 = 0x6012345612345678;
//   constexpr uint64_t kAddr2Larger = kAddr1 - std::numeric_limits<int32_t>::min();
//   auto result = AddressDifferenceAsInt32(kAddr1, kAddr2Larger);
//   ASSERT_THAT(result, HasNoError());
//   EXPECT_EQ(std::numeric_limits<int32_t>::min(), result.value());
//   result = AddressDifferenceAsInt32(kAddr1, kAddr2Larger + 1);
//   EXPECT_THAT(result, HasErrorWithMessage("Difference is larger than -2GB"));

//   // Result of the difference is positive; in the first case it just fits, the second case
//   // overflows.
//   constexpr uint64_t kAddr2Smaller = kAddr1 - std::numeric_limits<int32_t>::max();
//   result = AddressDifferenceAsInt32(kAddr1, kAddr2Smaller);
//   ASSERT_THAT(result, HasNoError());
//   EXPECT_EQ(std::numeric_limits<int32_t>::max(), result.value());
//   result = AddressDifferenceAsInt32(kAddr1, kAddr2Smaller - 1);
//   EXPECT_THAT(result, HasErrorWithMessage("Difference is larger than +2GB"));

//   // Result of the difference does not even fit into a int64. We handle that gracefully as well.
//   constexpr uint64_t kAddrHigh = 0xf234567812345678;
//   constexpr uint64_t kAddrLow = kAddrHigh - 0xe234567812345678;
//   result = AddressDifferenceAsInt32(kAddrHigh, kAddrLow);
//   EXPECT_THAT(result, HasErrorWithMessage("Difference is larger than +2GB"));
//   result = AddressDifferenceAsInt32(kAddrLow, kAddrHigh);
//   EXPECT_THAT(result, HasErrorWithMessage("Difference is larger than -2GB"));
// }

// class RelocateInstructionTest : public testing::Test {
//  protected:
//   void SetUp() override {
//     ORBIT_CHECK(cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle_) == CS_ERR_OK);
//     ORBIT_CHECK(cs_option(capstone_handle_, CS_OPT_DETAIL, CS_OPT_ON) == CS_ERR_OK);
//     instruction_ = cs_malloc(capstone_handle_);
//     ORBIT_CHECK(instruction_ != nullptr);
//   }

//   void Disassemble(const MachineCode& code) {
//     const uint8_t* code_pointer = code.GetResultAsVector().data();
//     size_t code_size = code.GetResultAsVector().size();
//     uint64_t disassemble_address = 0;
//     ORBIT_CHECK(cs_disasm_iter(capstone_handle_, &code_pointer, &code_size, &disassemble_address,
//                                instruction_));
//   }

//   void TearDown() override {
//     cs_free(instruction_, 1);
//     cs_close(&capstone_handle_);
//   }

//   cs_insn* instruction_ = nullptr;

//  private:
//   csh capstone_handle_ = 0;
// };

// TEST_F(RelocateInstructionTest, RipRelativeAddressing) {
//   MachineCode code;
//   constexpr int32_t kOffset = 0x969433;
//   // add qword ptr [rip + kOffset], 1
//   // Handled by "((instruction->detail->x86.modrm & 0xC7) == 0x05)" branch in 'RelocateInstruction'.
//   code.AppendBytes({0x48, 0x83, 0x05}).AppendImmediate32(kOffset).AppendBytes({0x01});
//   Disassemble(code);

//   constexpr uint64_t kOriginalAddress = 0x0100000000;
//   ErrorMessageOr<RelocatedInstruction> result =
//       RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress + kOffset - 0x123456);
//   ASSERT_THAT(result, HasValue());
//   // add qword ptr [rip + new_offset], 1      48 83 05 56 34 12 00 01
//   // new_offset is computed as
//   // old_absolute_address - new_address
//   // == (old_address + old_displacement) - (old_address + old_displacement - 0x123456)
//   // == 0x123456
//   EXPECT_THAT(result.value().code,
//               ElementsAreArray({0x48, 0x83, 0x05, 0x56, 0x34, 0x12, 0x00, 0x01}));
//   EXPECT_FALSE(result.value().position_of_absolute_address.has_value());

//   result =
//       RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress + kOffset + 0x123456);
//   ASSERT_THAT(result, HasValue());
//   // add qword ptr [rip + new_offset], 1      48 83 05 aa cb ed ff 01
//   // new_offset is computed as
//   // old_absolute_address - new_address
//   // == (old_address + old_displacement) - (old_address + old_displacement + 0x123456)
//   // == -0x123456 == 0xffedcbaa
//   EXPECT_THAT(result.value().code,
//               ElementsAreArray({0x48, 0x83, 0x05, 0xaa, 0xcb, 0xed, 0xff, 0x01}));
//   EXPECT_FALSE(result.value().position_of_absolute_address.has_value());

//   result = RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress - 0x7fff0000);
//   EXPECT_THAT(result,
//               HasErrorWithMessage(
//                   "While trying to relocate an instruction with rip relative addressing the "
//                   "target was out of range from the trampoline."));
// }

// TEST_F(RelocateInstructionTest, UnconditionalJumpTo8BitImmediate) {
//   MachineCode code;
//   constexpr int8_t kOffset = 0x08;
//   // jmp [rip + kOffset]
//   // Handled by "(instruction->detail->x86.opcode[0] == 0xeb)" branch in 'RelocateInstruction'.
//   code.AppendBytes({0xeb}).AppendImmediate8(kOffset);
//   Disassemble(code);

//   ErrorMessageOr<RelocatedInstruction> result =
//       RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
//   ASSERT_THAT(result, HasValue());
//   // jmp  [rip + 0]               ff 25 00 00 00 00
//   // absolute_address             0a 00 00 00 01 00 00 00
//   // original jump instruction ends on 0x0100000000 + 0x02. Adding kOffset (=8) yields 0x010000000a.
//   EXPECT_THAT(result.value().code, ElementsAreArray({0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
//                                                      0x00, 0x00, 0x01, 0x00, 0x00, 0x00}));
//   ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
//   EXPECT_EQ(6, result.value().position_of_absolute_address.value());
// }

// TEST_F(RelocateInstructionTest, UnconditionalJumpTo32BitImmediate) {
//   MachineCode code;
//   constexpr int32_t kOffset = 0x01020304;
//   // jmp [rip + kOffset]
//   // Handled by "(instruction->detail->x86.opcode[0] == 0xe9)" branch in 'RelocateInstruction'.
//   code.AppendBytes({0xe9}).AppendImmediate32(kOffset);
//   Disassemble(code);

//   ErrorMessageOr<RelocatedInstruction> result =
//       RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
//   ASSERT_THAT(result, HasValue());
//   // jmp  [rip + 0]               ff 25 00 00 00 00
//   // absolute_address             09 03 02 01 01 00 00 00
//   // original jump instruction ends on 0x0100000000 + 0x05. Adding kOffset yields 0x0101020309.
//   EXPECT_THAT(result.value().code, ElementsAreArray({0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x09, 0x03,
//                                                      0x02, 0x01, 0x01, 0x00, 0x00, 0x00}));
//   ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
//   EXPECT_EQ(6, result.value().position_of_absolute_address.value());
// }

// TEST_F(RelocateInstructionTest, CallInstructionIsNotSupported) {
//   MachineCode code;
//   constexpr int32_t kOffset = 0x01020304;
//   // call [rip + kOffset]
//   // Handled by "(instruction->detail->x86.opcode[0] == 0xe8)" branch in 'RelocateInstruction'.
//   code.AppendBytes({0xe8}).AppendImmediate32(kOffset);
//   Disassemble(code);

//   ErrorMessageOr<RelocatedInstruction> result =
//       RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
//   EXPECT_THAT(result, HasErrorWithMessage("Relocating a call instruction is not supported."));
// }

// TEST_F(RelocateInstructionTest, ConditionalJumpTo8BitImmediate) {
//   MachineCode code;
//   constexpr int8_t kOffset = 0x40;
//   // jno rip + kOffset
//   // Handled by "((instruction->detail->x86.opcode[0] & 0xf0) == 0x70)" branch in
//   // 'RelocateInstruction'.
//   code.AppendBytes({0x71}).AppendImmediate8(kOffset);
//   Disassemble(code);

//   ErrorMessageOr<RelocatedInstruction> result =
//       RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
//   ASSERT_THAT(result, HasValue());
//   // jo rip + 16                  70 0e
//   // jmp [rip + 6]                ff 25 00 00 00 00
//   // absolute_address             42 00 00 00 01 00 00 00
//   // original jump instruction ends on 0x0100000002 + 0x40 (kOffset) == 0x0100000042.
//   EXPECT_THAT(result.value().code,
//               ElementsAreArray({0x70, 0x0e, 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00,
//                                 0x00, 0x01, 0x00, 0x00, 0x00}));
//   ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
//   EXPECT_EQ(8, result.value().position_of_absolute_address.value());
// }

// TEST_F(RelocateInstructionTest, ConditionalJumpTo32BitImmediate) {
//   MachineCode code;
//   constexpr int32_t kOffset = 0x12345678;
//   // jno rip + kOffset           0f 80 78 56 34 12
//   // Handled by "(instruction->detail->x86.opcode[0] == 0x0f &&
//   //             (instruction->detail->x86.opcode[1] & 0xf0) == 0x80)"
//   // branch in 'RelocateInstruction'.
//   code.AppendBytes({0x0f, 0x80}).AppendImmediate32(kOffset);
//   Disassemble(code);

//   ErrorMessageOr<RelocatedInstruction> result =
//       RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
//   ASSERT_TRUE(result.has_value());
//   // jo rip + 16                  71 0e
//   // jmp [rip +6]                 ff 25 00 00 00 00
//   // absolute_address             7a 56 34 12 01 00 00 00
//   // original jump instruction ends on 0x0100000006 + 0x12345678 (kOffset) == 0x011234567e.
//   EXPECT_THAT(result.value().code,
//               ElementsAreArray({0x71, 0x0e, 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x56, 0x34,
//                                 0x12, 0x01, 0x00, 0x00, 0x00}));
//   ASSERT_TRUE(result.value().position_of_absolute_address.has_value());
//   EXPECT_EQ(8, result.value().position_of_absolute_address.value());
// }

// TEST_F(RelocateInstructionTest, LoopIsUnsupported) {
//   MachineCode code;
//   constexpr int8_t kOffset = 0x40;
//   // loopz rip + kOffset
//   // Handled by "((instruction->detail->x86.opcode[0] & 0xfc) == 0xe0)" branch in
//   // 'RelocateInstruction'.
//   code.AppendBytes({0xe1}).AppendImmediate8(kOffset);
//   Disassemble(code);

//   ErrorMessageOr<RelocatedInstruction> result =
//       RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
//   EXPECT_THAT(result, HasErrorWithMessage("Relocating a loop instruction is not supported."));
// }

// TEST_F(RelocateInstructionTest, TrivialTranslation) {
//   MachineCode code;
//   // nop
//   // Handled by "else" branch in 'RelocateInstruction' - instruction is just copied.
//   code.AppendBytes({0x90});
//   Disassemble(code);

//   ErrorMessageOr<RelocatedInstruction> result =
//       RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
//   ASSERT_THAT(result, HasValue());
//   EXPECT_THAT(result.value().code, ElementsAreArray({0x90}));
//   EXPECT_FALSE(result.value().position_of_absolute_address.has_value());
// }

// class InstrumentFunctionTest : public testing::Test {
//  protected:
//   void SetUp() override {
//     /* copybara:insert(b/237251106 injecting the library into the target process triggers some
//                        initilization code that check fails.)
//     GTEST_SKIP();
//     */
//     // Init Capstone disassembler.
//     cs_err error_code = cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle_);
//     ORBIT_CHECK(error_code == CS_ERR_OK);
//     error_code = cs_option(capstone_handle_, CS_OPT_DETAIL, CS_OPT_ON);
//     ORBIT_CHECK(error_code == CS_ERR_OK);

//     max_trampoline_size_ = GetMaxTrampolineSize();
//   }

//   void RunChild(int (*function_pointer)(), std::string_view function_name) {
//     function_name_ = function_name;

//     pid_ = fork();
//     ORBIT_CHECK(pid_ != -1);
//     if (pid_ == 0) {
//       prctl(PR_SET_PDEATHSIG, SIGTERM);

//       // Endless loops without side effects are UB and recent versions of clang optimize
//       // it away. Making `sum` volatile avoids that problem.
//       [[maybe_unused]] volatile uint64_t sum = 0;
//       while (true) {
//         sum += (*function_pointer)();
//       }
//     }
//   }

//   AddressRange GetFunctionAddressRangeOrDie() {
//     return GetFunctionAbsoluteAddressRangeOrDie(function_name_);
//   }

//   void PrepareInstrumentation(std::string_view entry_payload_function_name,
//                               std::string_view exit_payload_function_name) {
//     // Stop the child process using our tooling.
//     ORBIT_CHECK(AttachAndStopProcess(pid_).has_value());

//     auto library_path_or_error = GetTestLibLibraryPath();
//     ORBIT_CHECK(library_path_or_error.has_value());
//     std::filesystem::path library_path = std::move(library_path_or_error.value());

//     auto modules_or_error = orbit_module_utils::ReadModules(pid_);
//     ORBIT_CHECK(modules_or_error.has_value());
//     const std::vector<orbit_grpc_protos::ModuleInfo>& modules = modules_or_error.value();

//     // Inject the payload for the instrumentation.
//     auto library_handle_or_error = DlmopenInTracee(pid_, modules, library_path, RTLD_NOW,
//                                                    LinkerNamespace::kCreateNewNamespace);
//     ORBIT_CHECK(library_handle_or_error.has_value());
//     void* library_handle = library_handle_or_error.value();

//     auto entry_payload_function_address_or_error =
//         DlsymInTracee(pid_, modules, library_handle, entry_payload_function_name);
//     ORBIT_CHECK(entry_payload_function_address_or_error.has_value());
//     entry_payload_function_address_ =
//         absl::bit_cast<uint64_t>(entry_payload_function_address_or_error.value());

//     auto exit_payload_function_address_or_error =
//         DlsymInTracee(pid_, modules, library_handle, exit_payload_function_name);
//     ORBIT_CHECK(exit_payload_function_address_or_error.has_value());
//     exit_payload_function_address_ =
//         absl::bit_cast<uint64_t>(exit_payload_function_address_or_error.value());

//     // Get address of the function to instrument.
//     const AddressRange address_range_code = GetFunctionAddressRangeOrDie();
//     function_address_ = address_range_code.start;
//     const uint64_t size_of_function = address_range_code.end - address_range_code.start;

//     // Get memory for the trampoline.
//     auto trampoline_or_error =
//         AllocateMemoryForTrampolines(pid_, address_range_code, max_trampoline_size_);
//     ORBIT_CHECK(!trampoline_or_error.has_error());
//     trampoline_memory_ = std::move(trampoline_or_error.value());
//     trampoline_address_ = trampoline_memory_->GetAddress();

//     // Get memory for return trampoline and create the return trampoline.
//     auto return_trampoline_or_error = MemoryInTracee::Create(pid_, 0, GetReturnTrampolineSize());
//     ORBIT_CHECK(!return_trampoline_or_error.has_error());
//     return_trampoline_address_ = return_trampoline_or_error.value()->GetAddress();
//     auto result =
//         CreateReturnTrampoline(pid_, exit_payload_function_address_, return_trampoline_address_);
//     ORBIT_CHECK(!result.has_error());
//     ORBIT_CHECK(!return_trampoline_or_error.value()->EnsureMemoryExecutable().has_error());

//     // Copy the beginning of the function over into this process.
//     constexpr uint64_t kMaxFunctionBackupSize = 200;
//     const uint64_t bytes_to_copy = std::min(size_of_function, kMaxFunctionBackupSize);
//     ErrorMessageOr<std::vector<uint8_t>> function_backup =
//         ReadTraceesMemory(pid_, function_address_, bytes_to_copy);
//     ORBIT_CHECK(function_backup.has_value());
//     function_code_ = function_backup.value();
//   }

//   // Runs the child for a millisecond to assert it is still working fine, stops it, removes the
//   // instrumentation, restarts and stops it again.
//   void RestartAndRemoveInstrumentation() {
//     ORBIT_CHECK(!trampoline_memory_->EnsureMemoryExecutable().has_error());

//     MoveInstructionPointersOutOfOverwrittenCode(pid_, relocation_map_);

//     ORBIT_CHECK(!DetachAndContinueProcess(pid_).has_error());
//     std::this_thread::sleep_for(std::chrono::milliseconds(1));
//     ORBIT_CHECK(AttachAndStopProcess(pid_).has_value());

//     auto write_result_or_error = WriteTraceesMemory(pid_, function_address_, function_code_);
//     ORBIT_CHECK(!write_result_or_error.has_error());

//     ORBIT_CHECK(!DetachAndContinueProcess(pid_).has_error());
//     std::this_thread::sleep_for(std::chrono::milliseconds(1));
//     ORBIT_CHECK(AttachAndStopProcess(pid_).has_value());
//   }

//   void TearDown() override {
//     cs_close(&capstone_handle_);

//     // Detach and end child.
//     if (pid_ != -1) {
//       ORBIT_CHECK(!DetachAndContinueProcess(pid_).has_error());
//       kill(pid_, SIGKILL);
//       waitpid(pid_, nullptr, 0);
//     }
//   }

//   pid_t pid_ = -1;
//   csh capstone_handle_ = 0;
//   uint64_t max_trampoline_size_ = 0;
//   std::unique_ptr<MemoryInTracee> trampoline_memory_;
//   uint64_t trampoline_address_ = 0;
//   uint64_t return_trampoline_address_ = 0;
//   uint64_t entry_payload_function_address_ = 0;
//   uint64_t exit_payload_function_address_ = 0;

//   absl::flat_hash_map<uint64_t, uint64_t> relocation_map_;

//   std::string function_name_;
//   uint64_t function_address_ = 0;
//   std::vector<uint8_t> function_code_;
// };

// // Function with an ordinary compiler-synthesised prologue; performs some arithmetics. Most real
// // world functions will look like this (starting with pushing the stack frame...). Most functions
// // below are declared "naked", i.e. without the prologue and implemented entirely in assembly. This
// // is done to also cover edge cases.
// extern "C" __attribute__((noinline)) int DoSomething() {
//   std::random_device rd;
//   std::mt19937 gen(rd());
//   std::uniform_int_distribution<int> dis(1, 6);
//   std::vector<int> v(10);
//   std::generate(v.begin(), v.end(), [&]() { return dis(gen); });
//   int sum = std::accumulate(v.begin(), v.end(), 0);
//   return sum;
// }

// TEST_F(InstrumentFunctionTest, DoSomething) {
//   RunChild(&DoSomething, "DoSomething");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// TEST_F(InstrumentFunctionTest, CheckStackAlignedTo16Bytes) {
//   RunChild(&DoSomething, "DoSomething");
//   PrepareInstrumentation("EntryPayloadAlignedCopy", kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // We will not be able to instrument this - the function is just four bytes long and we need five
// // bytes to write a jump.
// extern "C" __attribute__((noinline, naked)) int TooShort() {
//   __asm__ __volatile__(
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// TEST_F(InstrumentFunctionTest, TooShort) {
// #if defined(ORBIT_COVERAGE_BUILD) || !defined(__clang__) || !defined(NDEBUG)
//   GTEST_SKIP();
// #endif
//   RunChild(&TooShort, "TooShort");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> result = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(result,
//               HasErrorWithMessage("Unable to disassemble enough of the function to instrument it"));
//   RestartAndRemoveInstrumentation();
// }

// // This function is just long enough to be instrumented (five bytes). It is also interesting in that
// // the return statement is copied into the trampoline and executed from there.
// extern "C" __attribute__((noinline, naked)) int LongEnough() {
//   __asm__ __volatile__(
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// TEST_F(InstrumentFunctionTest, LongEnough) {
//   RunChild(&LongEnough, "LongEnough");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // The rip relative address is translated to the new code position.
// extern "C" __attribute__((noinline, naked)) int RipRelativeAddressing() {
//   __asm__ __volatile__(
//       "movq 0x03(%%rip), %%rax\n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "ret \n\t"
//       ".quad 0x0102034200000000 \n\t"
//       :
//       :
//       :);
// }

// TEST_F(InstrumentFunctionTest, RipRelativeAddressing) {
//   RunChild(&RipRelativeAddressing, "RipRelativeAddressing");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // Unconditional jump to an 8-bit offset.
// extern "C" __attribute__((noinline, naked)) int UnconditionalJump8BitOffset() {
//   __asm__ __volatile__(
//       "jmp label_unconditional_jmp_8_bit \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "label_unconditional_jmp_8_bit: \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// TEST_F(InstrumentFunctionTest, UnconditionalJump8BitOffset) {
//   RunChild(&UnconditionalJump8BitOffset, "UnconditionalJump8BitOffset");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // Unconditional jump to a 32 bit offset.
// extern "C" __attribute__((noinline, naked)) int UnconditionalJump32BitOffset() {
//   __asm__ __volatile__(
//       "jmp label_unconditional_jmp_32_bit \n\t"
//       ".octa 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 \n\t"  // 256 bytes of zeros
//       "label_unconditional_jmp_32_bit: \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// TEST_F(InstrumentFunctionTest, UnconditionalJump32BitOffset) {
//   RunChild(&UnconditionalJump32BitOffset, "UnconditionalJump32BitOffset");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // The rip relative address is translated to the new code position.
// extern "C" __attribute__((noinline, naked)) int ConditionalJump8BitOffset() {
//   __asm__ __volatile__(
//       "jnz loop_label_jcc \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "loop_label_jcc: \n\t"
//       "xor %%eax, %%eax \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// TEST_F(InstrumentFunctionTest, ConditionalJump8BitOffset) {
//   RunChild(&ConditionalJump8BitOffset, "ConditionalJump8BitOffset");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // The rip relative address is translated to the new code position.
// extern "C" __attribute__((noinline, naked)) int ConditionalJump32BitOffset() {
//   __asm__ __volatile__(
//       "xor %%eax, %%eax \n\t"
//       "jnz label_jcc_32_bit \n\t"
//       "nop \n\t"
//       "ret \n\t"
//       ".octa 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 \n\t"  // 256 bytes of zeros
//       "label_jcc_32_bit: \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// TEST_F(InstrumentFunctionTest, ConditionalJump32BitOffset) {
//   RunChild(&ConditionalJump32BitOffset, "ConditionalJump32BitOffset");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // Function can not be instrumented since it uses the unsupported loop instruction.
// extern "C" __attribute__((noinline, naked)) int Loop() {
//   __asm__ __volatile__(
//       "mov $42, %%cx\n\t"
//       "loop_label:\n\t"
//       "loopnz loop_label\n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// TEST_F(InstrumentFunctionTest, Loop) {
// #if defined(ORBIT_COVERAGE_BUILD) || !defined(__clang__)
//   GTEST_SKIP();
// #endif
//   RunChild(&Loop, "Loop");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> result = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(result, HasErrorWithMessage("Relocating a loop instruction is not supported."));
//   RestartAndRemoveInstrumentation();
// }

// // Check-fails if any parameter is not zero.
// extern "C" __attribute__((noinline)) int CheckIntParameters(uint64_t p0, uint64_t p1, uint64_t p2,
//                                                             uint64_t p3, uint64_t p4, uint64_t p5,
//                                                             uint64_t p6, uint64_t p7) {
//   ORBIT_CHECK(p0 == 0 && p1 == 0 && p2 == 0 && p3 == 0 && p4 == 0 && p5 == 0 && p6 == 0 && p7 == 0);
//   return 0;
// }

// // This test and the tests below check for proper handling of parameters handed to the instrumented
// // function. The payload that is called before the instrumented function is executed clobbers the
// // respective set of registers. So the Check*Parameter methods can check if the backup worked
// // correctly.
// TEST_F(InstrumentFunctionTest, CheckIntParameters) {
//   function_name_ = "CheckIntParameters";
//   pid_ = fork();
//   ORBIT_CHECK(pid_ != -1);
//   if (pid_ == 0) {
//     prctl(PR_SET_PDEATHSIG, SIGTERM);

//     // Endless loops without side effects are UB and recent versions of clang optimize it away.
//     // Making `sum` volatile avoids that problem.
//     [[maybe_unused]] volatile uint64_t sum = 0;
//     while (true) {
//       sum += CheckIntParameters(0, 0, 0, 0, 0, 0, 0, 0);
//     }
//   }
//   PrepareInstrumentation("EntryPayloadClobberParameterRegisters", kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // Check-fails if any parameter is not zero.
// extern "C" __attribute__((noinline)) int CheckFloatParameters(float p0, float p1, float p2,
//                                                               float p3, float p4, float p5,
//                                                               float p6, float p7) {
//   ORBIT_CHECK(p0 == 0.f && p1 == 0.f && p2 == 0.f && p3 == 0.f && p4 == 0.f && p5 == 0.f &&
//               p6 == 0.f && p7 == 0.f);
//   return 0;
// }

// TEST_F(InstrumentFunctionTest, CheckFloatParameters) {
//   function_name_ = "CheckFloatParameters";
//   pid_ = fork();
//   ORBIT_CHECK(pid_ != -1);
//   if (pid_ == 0) {
//     prctl(PR_SET_PDEATHSIG, SIGTERM);

//     // Endless loops without side effects are UB and recent versions of clang optimize it away.
//     // Making `sum` volatile avoids that problem.
//     [[maybe_unused]] volatile uint64_t sum = 0;
//     while (true) {
//       sum += CheckFloatParameters(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
//     }
//   }
//   PrepareInstrumentation("EntryPayloadClobberXmmRegisters", kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // Check-fails if any parameter is not zero.
// extern "C" __attribute__((noinline)) int CheckM256iParameters(__m256i p0, __m256i p1, __m256i p2,
//                                                               __m256i p3, __m256i p4, __m256i p5,
//                                                               __m256i p6, __m256i p7) {
//   // ORBIT_CHECK(_mm256_extract_epi64(p0, 0) == 0 && _mm256_extract_epi64(p1, 0) == 0 &&
//   //             _mm256_extract_epi64(p2, 0) == 0 && _mm256_extract_epi64(p3, 0) == 0 &&
//   //             _mm256_extract_epi64(p4, 0) == 0 && _mm256_extract_epi64(p5, 0) == 0 &&
//   //             _mm256_extract_epi64(p6, 0) == 0 && _mm256_extract_epi64(p7, 0) == 0);
//   // return 0;
// }

// TEST_F(InstrumentFunctionTest, CheckM256iParameters) {
//   // function_name_ = "CheckM256iParameters";
//   // pid_ = fork();
//   // ORBIT_CHECK(pid_ != -1);
//   // if (pid_ == 0) {
//   //   prctl(PR_SET_PDEATHSIG, SIGTERM);

//   //   // Endless loops without side effects are UB and recent versions of clang optimize it away.
//   //   // Making `sum` volatile avoids that problem.
//   //   [[maybe_unused]] volatile uint64_t sum = 0;
//   //   while (true) {
//   //     sum +=
//   //         CheckM256iParameters(_mm256_set1_epi64x(0), _mm256_set1_epi64x(0), _mm256_set1_epi64x(0),
//   //                              _mm256_set1_epi64x(0), _mm256_set1_epi64x(0), _mm256_set1_epi64x(0),
//   //                              _mm256_set1_epi64x(0), _mm256_set1_epi64x(0));
//   //   }
//   // }
//   // PrepareInstrumentation("EntryPayloadClobberYmmRegisters", kExitPayloadFunctionName);
//   // ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//   //     pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//   //     return_trampoline_address_, capstone_handle_, relocation_map_);
//   // EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   // ErrorMessageOr<void> result =
//   //     InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//   //                        address_after_prologue_or_error.value(), trampoline_address_);
//   // EXPECT_THAT(result, HasNoError());
//   // RestartAndRemoveInstrumentation();
// }

// // Check-fails if any parameter is not zero.
// extern "C" __attribute__((noinline, ms_abi)) int CheckIntParametersMsAbi(uint64_t p0, uint64_t p1,
//                                                                          uint64_t p2, uint64_t p3) {
//   ORBIT_CHECK(p0 == 0 && p1 == 0 && p2 == 0 && p3 == 0);
//   return 0;
// }

// TEST_F(InstrumentFunctionTest, CheckIntParametersMsAbi) {
//   function_name_ = "CheckIntParametersMsAbi";
//   pid_ = fork();
//   ORBIT_CHECK(pid_ != -1);
//   if (pid_ == 0) {
//     prctl(PR_SET_PDEATHSIG, SIGTERM);

//     // Endless loops without side effects are UB and recent versions of clang optimize it away.
//     // Making `sum` volatile avoids that problem.
//     [[maybe_unused]] volatile uint64_t sum = 0;
//     while (true) {
//       sum += CheckIntParametersMsAbi(0, 0, 0, 0);
//     }
//   }
//   PrepareInstrumentation("EntryPayloadClobberParameterRegisters", kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // Check-fails if any parameter is not zero.
// extern "C" __attribute__((noinline, ms_abi)) int CheckFloatParametersMsAbi(float p0, float p1,
//                                                                            float p2, float p3) {
//   ORBIT_CHECK(p0 == 0.f && p1 == 0.f && p2 == 0.f && p3 == 0.f);
//   return 0;
// }

// TEST_F(InstrumentFunctionTest, CheckFloatParametersMsAbi) {
//   function_name_ = "CheckFloatParametersMsAbi";
//   pid_ = fork();
//   ORBIT_CHECK(pid_ != -1);
//   if (pid_ == 0) {
//     prctl(PR_SET_PDEATHSIG, SIGTERM);

//     // Endless loops without side effects are UB and recent versions of clang optimize it away.
//     // Making `sum` volatile avoids that problem.
//     [[maybe_unused]] volatile uint64_t sum = 0;
//     while (true) {
//       sum += CheckFloatParametersMsAbi(0.f, 0.f, 0.f, 0.f);
//     }
//   }
//   PrepareInstrumentation("EntryPayloadClobberXmmRegisters", kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// // This test guards against naively backing up x87 registers in the return trampoline when the
// // instrumented function doesn't use them to return values.
// TEST_F(InstrumentFunctionTest, CheckNoX87UnderflowInReturnTrampoline) {
//   function_name_ = "DoSomething";
//   pid_ = fork();
//   ORBIT_CHECK(pid_ != -1);
//   if (pid_ == 0) {
//     prctl(PR_SET_PDEATHSIG, SIGTERM);

//     // Reset bit 0 of the 16-bit x87 FPU Control Word, in order to unmask invalid-operation
//     // exception. If the return trampoline causes the underflow of the x87 register stack before
//     // masking the exception, the process will crash.
//     uint16_t control = 0;
//     __asm__ __volatile__("fnstcw %0\n\t" : "=m"(control) : :);
//     control &= 0xFE;
//     __asm__ __volatile__("fldcw %0\n\t" : : "m"(control) :);

//     // Endless loops without side effects are UB and recent versions of clang optimize it away.
//     // Making `sum` volatile avoids that problem.
//     [[maybe_unused]] volatile uint64_t sum = 0;
//     while (true) {
//       sum += DoSomething();
//     }
//   }
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> address_after_prologue_or_error = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(address_after_prologue_or_error, HasNoError());
//   ErrorMessageOr<void> result =
//       InstrumentFunction(pid_, function_address_, /*function_id=*/42,
//                          address_after_prologue_or_error.value(), trampoline_address_);
//   EXPECT_THAT(result, HasNoError());
//   RestartAndRemoveInstrumentation();
// }

// extern "C" __attribute__((noinline, naked)) int UnconditionalJump8BitOffsetBackToBeginning() {
//   __asm__ __volatile__(
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       ".byte 0xeb \n\t"  // jmp -7 (which is the first nop)
//       ".byte 0xf9 \n\t"
//       "xor %%eax, %%eax \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// // This will fail to create a trampoline since the function contains an unconditional jump to an
// // eight bit offset which points back into the first five bytes of the function.
// TEST_F(InstrumentFunctionTest, UnconditionalJump8BitOffsetBackToBeginning) {
// // Exclude gcc builds: the inline assembly above gets messed up by the compiler.
// #if defined(ORBIT_COVERAGE_BUILD) || !defined(__clang__) || !defined(NDEBUG)
//   GTEST_SKIP();
// #endif
//   RunChild(&UnconditionalJump8BitOffsetBackToBeginning,
//            "UnconditionalJump8BitOffsetBackToBeginning");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> result = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(result,
//               HasErrorWithMessage(
//                   "Failed to create trampoline since the function contains a jump back into"));
// }

// extern "C" __attribute__((noinline, naked)) int UnconditionalJump32BitOffsetBackToBeginning() {
//   __asm__ __volatile__(
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       ".byte 0xe9 \n\t"  // jmp -10 (which is the first nop)
//       ".long 0xfffffff6 \n\t"
//       "xor %%eax, %%eax \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// // This will fail to create a trampoline since the function contains an unconditional jump to a
// // 32 bit offset which points back into the first five bytes of the function.
// TEST_F(InstrumentFunctionTest, UnconditionalJump32BitOffsetBackToBeginning) {
// // Exclude gcc builds: the inline assembly above gets messed up by the compiler.
// #if defined(ORBIT_COVERAGE_BUILD) || !defined(__clang__) || !defined(NDEBUG)
//   GTEST_SKIP();
// #endif
//   RunChild(&UnconditionalJump32BitOffsetBackToBeginning,
//            "UnconditionalJump32BitOffsetBackToBeginning");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> result = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(result,
//               HasErrorWithMessage(
//                   "Failed to create trampoline since the function contains a jump back into"));
// }

// extern "C" __attribute__((noinline, naked)) int ConditionalJump8BitOffsetBackToBeginning() {
//   __asm__ __volatile__(
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       ".byte 0x70 \n\t"  // jo -7 (which is the first nop)
//       ".byte 0xf9 \n\t"
//       "xor %%eax, %%eax \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// // This will fail to create a trampoline since the function contains a conditional jump to an
// // eight bit offset which points back into the first five bytes of the function.
// TEST_F(InstrumentFunctionTest, ConditionalJump8BitOffsetBackToBeginning) {
// // Exclude gcc builds: the inline assembly above gets messed up by the compiler.
// #if defined(ORBIT_COVERAGE_BUILD) || !defined(__clang__) || !defined(NDEBUG)
//   GTEST_SKIP();
// #endif
//   RunChild(&ConditionalJump8BitOffsetBackToBeginning, "ConditionalJump8BitOffsetBackToBeginning");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> result = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(result,
//               HasErrorWithMessage(
//                   "Failed to create trampoline since the function contains a jump back into"));
// }

// extern "C" __attribute__((noinline, naked)) int ConditionalJump32BitOffsetBackToBeginning() {
//   __asm__ __volatile__(
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       ".byte 0x0f \n\t"  // jo -7 (which is the last nop)
//       ".byte 0x80 \n\t"
//       ".long 0xfffffff9 \n\t"
//       "xor %%eax, %%eax \n\t"
//       "ret \n\t"
//       :
//       :
//       :);
// }

// // This will fail to create a trampoline since the function contains a conditional jump to a
// // 32 bit offset which points back into the first five bytes of the function.
// TEST_F(InstrumentFunctionTest, ConditionalJump32BitOffsetBackToBeginning) {
// // Exclude gcc builds: the inline assembly above gets messed up by the compiler.
// #if defined(ORBIT_COVERAGE_BUILD) || !defined(__clang__) || !defined(NDEBUG)
//   GTEST_SKIP();
// #endif
//   RunChild(&ConditionalJump32BitOffsetBackToBeginning, "ConditionalJump32BitOffsetBackToBeginning");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> result = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(result,
//               HasErrorWithMessage(
//                   "Failed to create trampoline since the function contains a jump back into"));
// }

// extern "C" __attribute__((noinline, naked)) int LongConditionalJump32BitOffsetBackToBeginning() {
//   __asm__ __volatile__(
//       "xor %%eax, %%eax \n\t"
//       "ret \n\t"
//       ".octa 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 \n\t"  // 256 bytes of zeros
//       ".byte 0x0f \n\t"                                            // jo -263 (which is the ret)
//       ".byte 0x80 \n\t"
//       ".long 0xfffffef9 \n\t"
//       :
//       :
//       :);
// }

// // This will create a trampoline. The function contains a conditional jump to a
// // 32 bit offset which points back into the first five bytes of the function. However the jump is
// // occurring after the 200 byte limit and therefore it stays undetected.
// TEST_F(InstrumentFunctionTest, LongConditionalJump32BitOffsetBackToBeginning) {
//   RunChild(&LongConditionalJump32BitOffsetBackToBeginning,
//            "LongConditionalJump32BitOffsetBackToBeginning");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> result = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(result, HasNoError());
// }

// extern "C" __attribute__((noinline, naked)) int UnableToDisassembleBadInstruction() {
//   __asm__ __volatile__(
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "nop \n\t"
//       "ret \n\t"
//       ".byte 0x06 \n\t"  // bad instruction
//       ".byte 0x0f \n\t"  // jo -12 (which is the first nop)
//       ".byte 0x80 \n\t"
//       ".long 0xfffffff4 \n\t"
//       :
//       :
//       :);
// }

// // This will create a trampoline. There is a conditional jump back to the start but the disassembler
// // gets confused before it reaches this and so we don't detect it.
// TEST_F(InstrumentFunctionTest, UnableToDisassembleBadInstruction) {
//   RunChild(&UnableToDisassembleBadInstruction, "UnableToDisassembleBadInstruction");
//   PrepareInstrumentation(kEntryPayloadFunctionName, kExitPayloadFunctionName);
//   ErrorMessageOr<uint64_t> result = CreateTrampoline(
//       pid_, function_address_, function_code_, trampoline_address_, entry_payload_function_address_,
//       return_trampoline_address_, capstone_handle_, relocation_map_);
//   EXPECT_THAT(result, HasNoError());
// }

// }  // namespace orbit_user_space_instrumentation
