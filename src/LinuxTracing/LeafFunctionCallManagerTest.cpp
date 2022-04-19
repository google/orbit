// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "LeafFunctionCallManager.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "PerfEvent.h"
#include "PerfEventRecords.h"

using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Ge;
using ::testing::Lt;
using ::testing::Return;

using orbit_grpc_protos::Callstack;

namespace orbit_linux_tracing {

namespace {

class MockLibunwindstackMaps : public LibunwindstackMaps {
 public:
  MOCK_METHOD(unwindstack::MapInfo*, Find, (uint64_t), (override));
  MOCK_METHOD(unwindstack::Maps*, Get, (), (override));
  MOCK_METHOD(void, AddAndSort,
              (uint64_t, uint64_t, uint64_t, uint64_t, const std::string&, uint64_t), (override));
};

class MockLibunwindstackUnwinder : public LibunwindstackUnwinder {
 public:
  MOCK_METHOD(LibunwindstackResult, Unwind,
              (pid_t, unwindstack::Maps*, (const std::array<uint64_t, PERF_REG_X86_64_MAX>&),
               const void*, uint64_t, bool, size_t),
              (override));
  MOCK_METHOD(std::optional<bool>, HasFramePointerSet, (uint64_t, pid_t, unwindstack::Maps*),
              (override));
};

class LeafFunctionCallManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(maps_, Find(AllOf(Ge(kUprobesMapsStart), Lt(kUprobesMapsEnd))))
        .WillRepeatedly(Return(&kUprobesMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kTargetMapsStart), Lt(kTargetMapsEnd))))
        .WillRepeatedly(Return(&kTargetMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kNonExecutableMapsStart), Lt(kNonExecutableMapsEnd))))
        .WillRepeatedly(Return(&kNonExecutableMapInfo));
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

  static inline unwindstack::MapInfo kUprobesMapInfo{
      nullptr, nullptr, kUprobesMapsStart, kUprobesMapsEnd, 0, PROT_EXEC | PROT_READ, kUprobesName};

  static inline unwindstack::MapInfo kTargetMapInfo{
      nullptr, nullptr, kTargetMapsStart, kTargetMapsEnd, 0, PROT_EXEC | PROT_READ, kTargetName};

  static inline unwindstack::MapInfo kNonExecutableMapInfo{
      nullptr, nullptr,   kNonExecutableMapsStart, kNonExecutableMapsEnd,
      0,       PROT_READ, kNonExecutableName};

  static inline unwindstack::FrameData kFrame1{
      .pc = kTargetAddress1,
      .function_name = "foo",
      .function_offset = 0,
      .map_name = kTargetName,
  };

  static inline unwindstack::FrameData kFrame2{
      .pc = kTargetAddress2,
      .function_name = "bar",
      .function_offset = 0,
      .map_name = kTargetName,
  };

  static inline unwindstack::FrameData kFrame3{
      .pc = kTargetAddress3,
      .function_name = "baz",
      .function_offset = 0,
      .map_name = kTargetName,
  };

  static inline unwindstack::FrameData kNonExecutableFrame{
      .pc = kNonExecutableMapsStart,
      .function_name = "???",
      .function_offset = 0,
      .map_name = kNonExecutableName,
  };
};

CallchainSamplePerfEventData BuildFakeCallchainSamplePerfEventData(
    const std::vector<uint64_t>& callchain) {
  CallchainSamplePerfEventData event_data{
      .pid = 10,
      .tid = 11,
      .regs = make_unique_for_overwrite<perf_event_sample_regs_user_all>(),
      .data = make_unique_for_overwrite<char[]>(13)};
  event_data.SetIps(callchain);
  if (callchain.size() > 1) {
    // Set the first non-kernel address as IP.
    event_data.regs->ip = callchain[1];
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
  event_data.regs->bp = 2 * kStackDumpSize;
  event_data.regs->sp = 0;

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // The stack dump is too small, so we are only able to unwind the instruction
  // pointer.
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);

  // We expect `kStackDumpSize` here as size, as we do not want libunwindstack
  // to read out of bounds.
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, kStackDumpSize, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_EQ(Callstack::kStackTopForDwarfUnwindingTooSmall,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));
}

TEST_F(
    LeafFunctionCallManagerTest,
    PatchCallerOfLeafFunctionReturnsSuccessAndPatchesCallchainEvenIfStackDumpDoesContainFullCaller) {
  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEventData event_data = BuildFakeCallchainSamplePerfEventData(callchain);
  event_data.regs->bp = kStackDumpSize * 2;
  event_data.regs->sp = 0;

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // The stack dump is too small to fully contain the caller's frame, but large
  // enough to actually unwind the caller successfully.
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kFrame2);

  // We expect `kStackDumpSize` here as size, as we do not want libunwindstack
  // to read out of bounds.
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, kStackDumpSize, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(&kTargetMapInfo));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
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
  event_data.regs->bp = 2 * kStackDumpSize;
  event_data.regs->sp = 0;

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
  event_data.regs->bp = 2 * kStackDumpSize;
  event_data.regs->sp = 0;

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
  event_data.regs->bp = kStackDumpSize;
  event_data.regs->sp = 10;

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // Usually, we should get at least the instruction pointer as frame, even on unwinding errors.
  // However, we should also support empty callstacks and treat them as unwinding error.
  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{{}, unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_EQ(Callstack::kStackTopDwarfUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
  EXPECT_THAT(event_data.CopyOfIpsAsVector(), ElementsAreArray(callchain));

  ::testing::Mock::VerifyAndClearExpectations(&unwinder_);

  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // Unwinding errors could also result in non-executable code:
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kNonExecutableFrame);

  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_CALL(maps_, Find(kNonExecutableMapsStart))
      .Times(1)
      .WillOnce(Return(&kNonExecutableMapInfo));

  EXPECT_EQ(Callstack::kStackTopDwarfUnwindingError,
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
  event_data.regs->bp = kStackDumpSize;
  event_data.regs->sp = 10;

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // When libunwindstack reports more than two frames, there are no frame pointers.
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kFrame2);
  libunwindstack_callstack.push_back(kFrame3);

  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_EQ(Callstack::kFramePointerUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
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
  event_data.regs->bp = kStackDumpSize;
  event_data.regs->sp = 10;

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  // When libunwindstack reports exactly one frame (the ip), the innermost function has frame
  // pointers.
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);

  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
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
  event_data.regs->bp = kStackDumpSize;
  event_data.regs->sp = 10;

  unwindstack::Maps fake_maps{};
  EXPECT_CALL(maps_, Get()).WillRepeatedly(Return(&fake_maps));
  EXPECT_CALL(unwinder_, HasFramePointerSet(kTargetAddress1, _, &fake_maps))
      .Times(1)
      .WillOnce(Return(std::make_optional<bool>(false)));

  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kFrame2);

  EXPECT_CALL(unwinder_, Unwind(event_data.pid, &fake_maps, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(&kTargetMapInfo));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event_data, &maps_, &unwinder_));
  EXPECT_THAT(
      event_data.CopyOfIpsAsVector(),
      ElementsAre(kKernelAddress, kTargetAddress1, kTargetAddress2 + 1, kTargetAddress3 + 1));
  EXPECT_EQ(event_data.GetCallchainSize(), callchain.size() + 1);
}

}  // namespace orbit_linux_tracing
