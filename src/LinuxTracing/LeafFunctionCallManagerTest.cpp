// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <asm/perf_regs.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <unwindstack/Error.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>
#include <unwindstack/SharedString.h>
#include <unwindstack/Unwinder.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "LeafFunctionCallManager.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackMultipleOfflineAndProcessMemory.h"
#include "LibunwindstackUnwinder.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "PerfEvent.h"
#include "PerfEventRecords.h"
#include "TestUtils/SaveRangeFromArg.h"
#include "unwindstack/MachineX86_64.h"
#include "unwindstack/Regs.h"
#include "unwindstack/RegsX86_64.h"

using ::orbit_test_utils::SaveRangeFromArg;
using ::testing::_;
using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Lt;
using ::testing::NotNull;
using ::testing::Property;
using ::testing::Return;

using orbit_grpc_protos::Callstack;

namespace orbit_linux_tracing {

namespace {

constexpr uint64_t kTotalNumOfRegisters =
    sizeof(perf_event_sample_regs_user_all) / sizeof(uint64_t);

class MockLibunwindstackMaps : public LibunwindstackMaps {
 public:
  MOCK_METHOD(std::shared_ptr<unwindstack::MapInfo>, Find, (uint64_t), (override));
  MOCK_METHOD(unwindstack::Maps*, Get, (), (override));
  MOCK_METHOD(void, AddAndSort, (uint64_t, uint64_t, uint64_t, uint64_t, std::string_view),
              (override));
};

class MockLibunwindstackUnwinder : public LibunwindstackUnwinder {
 public:
  MOCK_METHOD(LibunwindstackResult, Unwind,
              (pid_t, unwindstack::Maps*, (const std::array<uint64_t, PERF_REG_X86_64_MAX>&),
               absl::Span<const StackSliceView>, bool, size_t),
              (override));
  MOCK_METHOD(std::optional<bool>, HasFramePointerSet, (uint64_t, pid_t, unwindstack::Maps*),
              (override));
};

class LeafFunctionCallManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(maps_, Find(AllOf(Ge(kUprobesMapsStart), Lt(kUprobesMapsEnd))))
        .WillRepeatedly(Return(kUprobesMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kTargetMapsStart), Lt(kTargetMapsEnd))))
        .WillRepeatedly(Return(kTargetMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kNonExecutableMapsStart), Lt(kNonExecutableMapsEnd))))
        .WillRepeatedly(Return(kNonExecutableMapInfo));
  }

  void TearDown() override {}

  MockLibunwindstackMaps maps_;
  MockLibunwindstackUnwinder unwinder_;

  static constexpr uint16_t kStackDumpSize = 128;

  LeafFunctionCallManager leaf_function_call_manager_{kStackDumpSize};

  static constexpr uint64_t kUprobesMapsStart = 42;
  static constexpr uint64_t kUprobesMapsEnd = 84;

  static constexpr uint64_t kTargetMapsStart = 100;
  static constexpr uint64_t kTargetMapsEnd = 200;

  static constexpr uint64_t kNonExecutableMapsStart = 500;
  static constexpr uint64_t kNonExecutableMapsEnd = 600;

  static constexpr uint64_t kKernelAddress = 11;

  static constexpr uint64_t kTargetAddress1 = 100;
  static constexpr uint64_t kTargetAddress2 = 200;
  static constexpr uint64_t kTargetAddress3 = 300;

  static inline const std::string kUprobesName = "[uprobes]";
  static inline const std::string kTargetName = "target";
  static inline const std::string kNonExecutableName = "data";

  static inline const std::shared_ptr<unwindstack::MapInfo> kUprobesMapInfo =
      unwindstack::MapInfo::Create(kUprobesMapsStart, kUprobesMapsEnd, 0, PROT_EXEC | PROT_READ,
                                   kUprobesName);

  static inline const std::shared_ptr<unwindstack::MapInfo> kTargetMapInfo =
      unwindstack::MapInfo::Create(kTargetMapsStart, kTargetMapsEnd, 0, PROT_EXEC | PROT_READ,
                                   kTargetName);

  static inline const std::shared_ptr<unwindstack::MapInfo> kNonExecutableMapInfo =
      unwindstack::MapInfo::Create(kNonExecutableMapsStart, kNonExecutableMapsEnd, 0, PROT_READ,
                                   kNonExecutableName);

  static inline unwindstack::FrameData kFrame1{
      .pc = kTargetAddress1,
      .function_name = "foo",
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  static inline unwindstack::FrameData kFrame2{
      .pc = kTargetAddress2,
      .function_name = "bar",
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  static inline unwindstack::FrameData kFrame3{
      .pc = kTargetAddress3,
      .function_name = "baz",
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  static inline unwindstack::FrameData kNonExecutableFrame{
      .pc = kNonExecutableMapsStart,
      .function_name = "???",
      .function_offset = 0,
      .map_info = kNonExecutableMapInfo,
  };
};

CallchainSamplePerfEventData BuildFakeCallchainSamplePerfEventData(
    absl::Span<const uint64_t> callchain) {
  CallchainSamplePerfEventData event_data{
      .pid = 10,
      .tid = 11,
      .regs = make_unique_for_overwrite<uint64_t[]>(kTotalNumOfRegisters),
      .data = make_unique_for_overwrite<uint8_t[]>(13)};

  event_data.SetIps(callchain);

  if (callchain.size() > 1) {
    // Set the first non-kernel address as IP.
    perf_event_sample_regs_user_all regs{};
    regs.ip = callchain[1];
    std::memcpy(event_data.regs.get(), &regs, sizeof(regs));
  }
  return event_data;
}

}  // namespace

TEST_F(LeafFunctionCallManagerTest, PatchCallerOfLeafFunctionReturnsErrorOnTooSmallStackSamples) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  perf_event_sample_regs_user_all regs{};
  regs.bp = 2 * kStackDumpSize;
  regs.sp = 0;
  regs.ip = kTargetAddress1;
  std::memcpy(event_data.regs.get(), &regs, sizeof(regs));

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // The stack dump is too small, so we are only able to unwind the instruction
  // pointer.
  unwindstack::RegsX86_64 libunwindstack_regs{};
  libunwindstack_regs[unwindstack::X86_64_REG_RBP] = event_data.GetRegisters().bp;
  libunwindstack_regs[unwindstack::X86_64_REG_RSP] = event_data.GetRegisters().sp;
  libunwindstack_regs.set_pc(event_data.GetRegisters().ip);

  std::vector<StackSliceView> actual_stack_slices;
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, _, 1))
      .Times(1)
      .WillOnce(
          DoAll(orbit_test_utils::SaveRangeFromArg<3>(&actual_stack_slices),
                Return(LibunwindstackResult{
                    {kFrame1}, libunwindstack_regs, unwindstack::ErrorCode::ERROR_INVALID_MAP})));

  EXPECT_EQ(Callstack::kStackTopForDwarfUnwindingTooSmall,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));

  // We expect `kStackDumpSize` here as size, as we do not want libunwindstack
  // to read out of bounds.
  EXPECT_THAT(actual_stack_slices,
              ElementsAre(AllOf(Property("start_address", &StackSliceView::start_address,
                                         Eq(event_data.GetRegisters().sp)),
                                Property("size", &StackSliceView::size, Eq(kStackDumpSize)),
                                Property("data", &StackSliceView::data, NotNull()))));
  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));
}

TEST_F(
    LeafFunctionCallManagerTest,
    PatchCallerOfLeafFunctionReturnsSuccessAndPatchesCallchainEvenIfStackDumpDoesNotFullyContainCaller) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  perf_event_sample_regs_user_all regs{};
  regs.bp = 2 * kStackDumpSize;
  regs.sp = 0;
  regs.ip = kTargetAddress1;
  std::memcpy(event_data.regs.get(), &regs, sizeof(regs));

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // The stack dump is too small to fully contain the caller's frame, but large
  // enough to actually unwind the caller successfully.
  unwindstack::RegsX86_64 libunwindstack_regs{};
  libunwindstack_regs[unwindstack::X86_64_REG_RBP] = event_data.GetRegisters().bp;
  libunwindstack_regs[unwindstack::X86_64_REG_RSP] = kStackDumpSize;
  libunwindstack_regs.set_pc(kTargetAddress2 + 1);

  std::vector<StackSliceView> actual_stack_slices;
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, _, 1))
      .Times(1)
      .WillOnce(
          DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                Return(LibunwindstackResult{
                    {kFrame1}, libunwindstack_regs, unwindstack::ErrorCode::ERROR_INVALID_MAP})));

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
  // We expect `kStackDumpSize` here as size, as we do not want libunwindstack
  // to read out of bounds.
  EXPECT_THAT(actual_stack_slices,
              ElementsAre(AllOf(Property("start_address", &StackSliceView::start_address,
                                         Eq(event_data.GetRegisters().sp)),
                                Property("size", &StackSliceView::size, Eq(kStackDumpSize)),
                                Property("data", &StackSliceView::data, NotNull()))));
  EXPECT_THAT(
      event_data.CopyOfIpsAsVector(),
      ElementsAre(kKernelAddress, kTargetAddress1, kTargetAddress2 + 1, kTargetAddress3 + 1));
  EXPECT_EQ(event_data.GetCallchainSize(), callchain.size() + 1);
}

TEST_F(LeafFunctionCallManagerTest,
       PatchCallerOfLeafFunctionReturnsSucceedsOnNonLeafFunctionEvenIfStackSampleTooSmall) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  perf_event_sample_regs_user_all regs{};
  regs.bp = 2 * kStackDumpSize;
  regs.sp = 0;
  regs.ip = kTargetAddress1;
  std::memcpy(event_data.regs.get(), &regs, sizeof(regs));

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));

  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(true)));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest,
       PatchCallerOfLeafFunctionReturnsErrorOnFramePointerDetectionFailure) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  perf_event_sample_regs_user_all regs{};
  regs.bp = 2 * kStackDumpSize;
  regs.sp = 0;
  regs.ip = kTargetAddress1;
  std::memcpy(event_data.regs.get(), &regs, sizeof(regs));

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));

  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::nullopt));

  EXPECT_EQ(Callstack::kStackTopDwarfUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest, PatchCallerOfLeafFunctionReturnsErrorOnUnwindingErrors) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  perf_event_sample_regs_user_all regs{};
  regs.bp = kStackDumpSize / 2;
  regs.sp = 10;
  regs.ip = kTargetAddress1;
  std::memcpy(event_data.regs.get(), &regs, sizeof(regs));

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  unwindstack::RegsX86_64 libunwindstack_regs{};
  libunwindstack_regs[unwindstack::X86_64_REG_RBP] = event_data.GetRegisters().bp;
  libunwindstack_regs[unwindstack::X86_64_REG_RSP] = 20;
  libunwindstack_regs.set_pc(kNonExecutableMapsStart);

  std::vector<StackSliceView> actual_stack_slices;
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, _, 1))
      .Times(1)
      .WillOnce(DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                      Return(LibunwindstackResult{
                          {}, libunwindstack_regs, unwindstack::ErrorCode::ERROR_INVALID_MAP})));

  EXPECT_EQ(Callstack::kStackTopDwarfUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));

  // Usually, we should get at least the instruction pointer as frame, even on unwinding errors.
  // However, we should also support empty callstacks and treat them as unwinding error.
  EXPECT_THAT(actual_stack_slices,
              ElementsAre(AllOf(
                  Property("start_address", &StackSliceView::start_address,
                           Eq(event_data.GetRegisters().sp)),
                  Property("size", &StackSliceView::size,
                           Eq(event_data.GetRegisters().bp - event_data.GetRegisters().sp + 16)),
                  Property("data", &StackSliceView::data, NotNull()))));

  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));

  ::testing::Mock::VerifyAndClearExpectations(&unwinder_);

  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // Unwinding errors could also result in non-executable code:
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);

  libunwindstack_regs[unwindstack::X86_64_REG_RBP] = event_data.GetRegisters().bp;
  libunwindstack_regs[unwindstack::X86_64_REG_RSP] = 20;
  libunwindstack_regs.set_pc(kNonExecutableMapsStart);

  actual_stack_slices.clear();
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, _, 1))
      .Times(1)
      .WillOnce(
          DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                Return(LibunwindstackResult{
                    {kFrame1}, libunwindstack_regs, unwindstack::ErrorCode::ERROR_INVALID_MAP})));

  EXPECT_CALL(maps_, Find(kNonExecutableMapsStart))
      .Times(1)
      .WillOnce(Return(kNonExecutableMapInfo));

  EXPECT_EQ(Callstack::kStackTopDwarfUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
  EXPECT_THAT(actual_stack_slices,
              ElementsAre(AllOf(
                  Property("start_address", &StackSliceView::start_address,
                           Eq(event_data.GetRegisters().sp)),
                  Property("size", &StackSliceView::size,
                           Eq(event_data.GetRegisters().bp - event_data.GetRegisters().sp + 16)),
                  Property("data", &StackSliceView::data, NotNull()))));
  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest, PatchCallerOfLeafFunctionReturnsErrorOnNoFramePointerInRbp) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  // bp < sp indicates that bp was used as general purpose register
  perf_event_sample_regs_user_all regs{};
  regs.bp = 1;
  regs.sp = 10;
  regs.ip = kTargetAddress1;
  std::memcpy(event_data.regs.get(), &regs, sizeof(regs));

  EXPECT_EQ(Callstack::kFramePointerUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest, PatchCallerOfLeafFunctionReturnsErrorOnNoFramePointers) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  event_data.regs.get()[6] = kStackDumpSize / 10;
  event_data.regs.get()[7] = 10;

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  unwindstack::RegsX86_64 libunwindstack_regs{};
  libunwindstack_regs[unwindstack::X86_64_REG_RBP] = event_data.GetRegisters().bp - 10;
  libunwindstack_regs[unwindstack::X86_64_REG_RSP] = 20;
  libunwindstack_regs.set_pc(kTargetAddress2);

  std::vector<StackSliceView> actual_stack_slices;
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, _, 1))
      .Times(1)
      .WillOnce(
          DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                Return(LibunwindstackResult{
                    {kFrame1}, libunwindstack_regs, unwindstack::ErrorCode::ERROR_INVALID_MAP})));

  EXPECT_EQ(Callstack::kFramePointerUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));

  EXPECT_THAT(actual_stack_slices,
              ElementsAre(AllOf(
                  Property("start_address", &StackSliceView::start_address,
                           Eq(event_data.GetRegisters().sp)),
                  Property("size", &StackSliceView::size,
                           Eq(event_data.GetRegisters().bp - event_data.GetRegisters().sp + 16)),
                  Property("data", &StackSliceView::data, NotNull()))));
  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest,
       PatchCallerOfLeafFunctionReturnsSuccessAndKeepsCallchainUntouchedOnNonLeafFunctions) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  perf_event_sample_regs_user_all regs{};
  regs.bp = kStackDumpSize / 2;
  regs.sp = 10;
  regs.ip = kTargetAddress1;
  std::memcpy(event_data.regs.get(), &regs, sizeof(regs));

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // When libunwindstack reports a different, but valid, rbp after unwinding,
  // the innermost function has frame pointers.
  unwindstack::RegsX86_64 libunwindstack_regs{};
  libunwindstack_regs[unwindstack::X86_64_REG_RBP] = event_data.GetRegisters().bp + 10;
  libunwindstack_regs[unwindstack::X86_64_REG_RSP] = 20;
  libunwindstack_regs.set_pc(kTargetAddress2);

  std::vector<StackSliceView> actual_stack_slices;
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, _, 1))
      .Times(1)
      .WillOnce(
          DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                Return(LibunwindstackResult{
                    {kFrame1}, libunwindstack_regs, unwindstack::ErrorCode::ERROR_INVALID_MAP})));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));

  EXPECT_THAT(actual_stack_slices,
              ElementsAre(AllOf(
                  Property("start_address", &StackSliceView::start_address,
                           Eq(event_data.GetRegisters().sp)),
                  Property("size", &StackSliceView::size,
                           Eq(event_data.GetRegisters().bp - event_data.GetRegisters().sp + 16)),
                  Property("data", &StackSliceView::data, NotNull()))));

  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest,
       PatchCallerOfLeafFunctionReturnsSuccessAndPatchesCallchainOnLeafFunctions) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  perf_event_sample_regs_user_all regs{};
  regs.bp = kStackDumpSize / 2;
  regs.sp = 10;
  regs.ip = kTargetAddress1;
  std::memcpy(event_data.regs.get(), &regs, sizeof(regs));

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  unwindstack::RegsX86_64 libunwindstack_regs{};
  libunwindstack_regs[unwindstack::X86_64_REG_RBP] = event_data.GetRegisters().bp;
  libunwindstack_regs[unwindstack::X86_64_REG_RSP] = 20;
  libunwindstack_regs.set_pc(kTargetAddress2 + 1);

  std::vector<StackSliceView> actual_stack_slices;
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, _, 1))
      .Times(1)
      .WillOnce(
          DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                Return(LibunwindstackResult{
                    {kFrame1}, libunwindstack_regs, unwindstack::ErrorCode::ERROR_INVALID_MAP})));

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));

  EXPECT_THAT(actual_stack_slices,
              ElementsAre(AllOf(
                  Property("start_address", &StackSliceView::start_address,
                           Eq(event_data.GetRegisters().sp)),
                  Property("size", &StackSliceView::size,
                           Eq(event_data.GetRegisters().bp - event_data.GetRegisters().sp + 16)),
                  Property("data", &StackSliceView::data, NotNull()))));

  EXPECT_THAT(
      event_data.CopyOfIpsAsVector(),
      ElementsAre(kKernelAddress, kTargetAddress1, kTargetAddress2 + 1, kTargetAddress3 + 1));
  EXPECT_EQ(event_data.GetCallchainSize(), callchain.size() + 1);
}

}  // namespace orbit_linux_tracing
