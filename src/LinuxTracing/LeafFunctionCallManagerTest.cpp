// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>

#include "LeafFunctionCallManager.h"

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
      nullptr, kUprobesMapsStart, kUprobesMapsEnd, 0, PROT_EXEC | PROT_READ, kUprobesName};

  static inline unwindstack::MapInfo kTargetMapInfo{nullptr, kTargetMapsStart,      kTargetMapsEnd,
                                                    0,       PROT_EXEC | PROT_READ, kTargetName};

  static inline unwindstack::MapInfo kNonExecutableMapInfo{
      nullptr, kNonExecutableMapsStart, kNonExecutableMapsEnd, 0, PROT_READ, kNonExecutableName};

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

}  // namespace

TEST_F(LeafFunctionCallManagerTest, PatchCallerOfLeafFunctionReturnsErrorOnSmallStackSamples) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEvent event{callchain.size(), kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;
  event.ips = callchain;

  event.regs.bp = 2 * kStackDumpSize;
  event.regs.sp = 0;

  EXPECT_EQ(Callstack::kStackTopForDwarfUnwindingTooSmall,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event, &maps_, &unwinder_));
  EXPECT_THAT(event.ips, ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest, PatchCallerOfLeafFunctionReturnsErrorOnUnwindingErrors) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEvent event{callchain.size(), kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;
  event.ips = callchain;
  event.regs.bp = kStackDumpSize;
  event.regs.sp = 10;

  EXPECT_CALL(maps_, Get).WillRepeatedly(Return(nullptr));

  // Usually, we should get at least the instruction pointer as frame, even on unwinding errors.
  // However, we should also support empty callstacks and treat them as unwinding error.
  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{{}, unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_EQ(Callstack::kStackTopDwarfUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event, &maps_, &unwinder_));
  EXPECT_THAT(event.ips, ElementsAreArray(callchain));

  ::testing::Mock::VerifyAndClearExpectations(&unwinder_);

  // Unwinding errors could also result in non-executable code:
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kNonExecutableFrame);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_CALL(maps_, Find(kNonExecutableMapsStart))
      .Times(1)
      .WillOnce(Return(&kNonExecutableMapInfo));

  EXPECT_EQ(Callstack::kStackTopDwarfUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event, &maps_, &unwinder_));
  EXPECT_THAT(event.ips, ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest, PatchCallerOfLeafFunctionReturnsErrorOnNoFramePointers) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEvent event{callchain.size(), kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;
  event.ips = callchain;
  event.regs.bp = kStackDumpSize;
  event.regs.sp = 10;

  EXPECT_CALL(maps_, Get).WillRepeatedly(Return(nullptr));

  // When libunwindstack reports more than two frames, there are no frame pointers.
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kFrame2);
  libunwindstack_callstack.push_back(kFrame3);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_EQ(Callstack::kFramePointerUnwindingError,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event, &maps_, &unwinder_));
  EXPECT_THAT(event.ips, ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest,
       PatchCallerOfLeafFunctionReturnsSuccessAndKeepsCallchainUntouchedOnNonLeafFunctions) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress2 + 1);
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEvent event{callchain.size(), kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;
  event.ips = callchain;
  event.regs.bp = kStackDumpSize;
  event.regs.sp = 10;

  EXPECT_CALL(maps_, Get).WillRepeatedly(Return(nullptr));

  // When libunwindstack reports exactly one frame (the ip), the innermost function has frame
  // pointers.
  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event, &maps_, &unwinder_));
  EXPECT_THAT(event.ips, ElementsAreArray(callchain));
}

TEST_F(LeafFunctionCallManagerTest,
       PatchCallerOfLeafFunctionReturnsSuccessAndPatchesCallchainOnLeafFunctions) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kTargetAddress3 + 1);

  CallchainSamplePerfEvent event{callchain.size(), kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;
  event.ips = callchain;
  event.regs.bp = kStackDumpSize;
  event.regs.sp = 10;

  EXPECT_CALL(maps_, Get).WillRepeatedly(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kFrame2);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackDumpSize - 10, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_INVALID_MAP}));

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(&kTargetMapInfo));

  EXPECT_EQ(Callstack::kComplete,
            leaf_function_call_manager_.PatchCallerOfLeafFunction(&event, &maps_, &unwinder_));
  EXPECT_THAT(event.ips, ElementsAre(kKernelAddress, kTargetAddress1, kTargetAddress2 + 1,
                                     kTargetAddress3 + 1));
  EXPECT_EQ(event.GetCallchainSize(), callchain.size() + 1);
}
}  // namespace orbit_linux_tracing