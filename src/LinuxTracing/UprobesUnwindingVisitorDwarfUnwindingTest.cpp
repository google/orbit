// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <google/protobuf/stubs/port.h>
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <unwindstack/Error.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Unwinder.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "LibunwindstackMultipleOfflineAndProcessMemory.h"
#include "LibunwindstackUnwinder.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "MockTracerListener.h"
#include "PerfEvent.h"
#include "PerfEventRecords.h"
#include "TestUtils/SaveRangeFromArg.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesUnwindingVisitor.h"
#include "UprobesUnwindingVisitorTestCommon.h"
#include "unwindstack/SharedString.h"

using orbit_test_utils::SaveRangeFromArg;

namespace orbit_linux_tracing {

class UprobesUnwindingVisitorDwarfUnwindingTestBase : public ::testing::Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(maps_, Find(::testing::AllOf(::testing::Ge(kUprobesMapsStart),
                                             ::testing::Lt(kUprobesMapsEnd))))
        .WillRepeatedly(::testing::Return(kUprobesMapInfo));

    EXPECT_CALL(maps_, Find(::testing::AllOf(::testing::Ge(kTargetMapsStart),
                                             ::testing::Lt(kTargetMapsEnd))))
        .WillRepeatedly(::testing::Return(kTargetMapInfo));

    EXPECT_CALL(maps_, Find(::testing::AllOf(::testing::Ge(kNonExecutableMapsStart),
                                             ::testing::Lt(kNonExecutableMapsEnd))))
        .WillRepeatedly(::testing::Return(kNonExecutableMapInfo));
  }

  static constexpr uint32_t kStackDumpSize = 128;
  MockTracerListener listener_;
  UprobesFunctionCallManager function_call_manager_;
  MockUprobesReturnAddressManager return_address_manager_{nullptr};
  MockLibunwindstackMaps maps_;
  MockLibunwindstackUnwinder unwinder_;
  MockLeafFunctionCallManager leaf_function_call_manager_{kStackDumpSize};

  static inline const std::string kUserSpaceLibraryName = "/path/to/library.so";

  static constexpr uint64_t kUserSpaceLibraryMapsStart = 0xCCCCCCCCCCCCCC00LU;
  static constexpr uint64_t kUserSpaceLibraryMapsEnd = 0xCCCCCCCCCCCCCCFFLU;
  const std::shared_ptr<unwindstack::MapInfo> kUserSpaceLibraryMapInfo =
      unwindstack::MapInfo::Create(kUserSpaceLibraryMapsStart, kUserSpaceLibraryMapsEnd, 0,
                                   PROT_EXEC | PROT_READ, kUserSpaceLibraryName);

  static constexpr uint64_t kUserSpaceLibraryAddress = kUserSpaceLibraryMapsStart;
  const std::string kUserSpaceLibraryFunctionName = "payload";
  const unwindstack::FrameData kUserSpaceLibraryFrame{
      .pc = kUserSpaceLibraryAddress,
      .function_name = kUserSpaceLibraryFunctionName,
      .function_offset = 0,
      .map_info = kUserSpaceLibraryMapInfo,
  };

  static constexpr uint64_t kEntryTrampolineAddress = 0xAAAAAAAAAAAAAA00LU;
  const std::string kEntryTrampolineFunctionName = "entry_trampoline";
  const std::shared_ptr<unwindstack::MapInfo> kEntryTrampolineMapInfo =
      unwindstack::MapInfo::Create(kEntryTrampolineAddress, kEntryTrampolineAddress + 0x1000, 0,
                                   PROT_EXEC | PROT_READ, "");
  const unwindstack::FrameData kEntryTrampolineFrame{
      .pc = kEntryTrampolineAddress,
      .function_name = kEntryTrampolineFunctionName,
      .function_offset = 0,
      .map_info = kEntryTrampolineMapInfo,
  };

  static constexpr uint64_t kReturnTrampolineAddress = 0xBBBBBBBBBBBBBB00LU;
  const std::string kReturnTrampolineFunctionName = "return_trampoline";
  const std::shared_ptr<unwindstack::MapInfo> kReturnTrampolineMapInfo =
      unwindstack::MapInfo::Create(kReturnTrampolineAddress, kReturnTrampolineAddress + 0x1000, 0,
                                   PROT_EXEC | PROT_READ, "");
  const unwindstack::FrameData kReturnTrampolineFrame{
      .pc = kReturnTrampolineAddress,
      .function_name = kReturnTrampolineFunctionName,
      .function_offset = 0,
      .map_info = kReturnTrampolineMapInfo,
  };

  class FakeUserSpaceInstrumentationAddresses : public UserSpaceInstrumentationAddresses {
   public:
    [[nodiscard]] bool IsInEntryTrampoline(uint64_t address) const override {
      return address == kEntryTrampolineAddress || address == kEntryTrampolineAddress + 1;
    }
    [[nodiscard]] bool IsInReturnTrampoline(uint64_t address) const override {
      return address == kReturnTrampolineAddress || address == kReturnTrampolineAddress + 1;
    }
    [[nodiscard]] std::string_view GetInjectedLibraryMapName() const override {
      return kUserSpaceLibraryName;
    }
  } user_space_instrumentation_addresses_;

  std::map<uint64_t, uint64_t> absolute_address_to_size_of_functions_to_stop_at_{};

  UprobesUnwindingVisitor visitor_{&listener_,
                                   &function_call_manager_,
                                   &return_address_manager_,
                                   &maps_,
                                   &unwinder_,
                                   &leaf_function_call_manager_,
                                   &user_space_instrumentation_addresses_,
                                   &absolute_address_to_size_of_functions_to_stop_at_};

  const std::string kUprobesName = "[uprobes]";
  static constexpr uint64_t kUprobesMapsStart = 0x7FFFFFFFE000;
  static constexpr uint64_t kUprobesMapsEnd = 0x7FFFFFFFE001;
  const std::shared_ptr<unwindstack::MapInfo> kUprobesMapInfo = unwindstack::MapInfo::Create(
      kUprobesMapsStart, kUprobesMapsEnd, 0, PROT_EXEC | PROT_READ, kUprobesName);

  const unwindstack::FrameData kUprobesFrame1{
      .pc = kUprobesMapsStart,
      .function_name = "uprobe",
      .function_offset = 0,
      .map_info = kUprobesMapInfo,
  };

  const unwindstack::FrameData kUprobesFrame2{
      .pc = kUprobesMapsStart + 1,
      .function_name = "uprobe",
      .function_offset = 0,
      .map_info = kUprobesMapInfo,
  };

  const std::string kTargetName = "target";
  static constexpr uint64_t kTargetMapsStart = 100;
  static constexpr uint64_t kTargetMapsEnd = 400;
  const std::shared_ptr<unwindstack::MapInfo> kTargetMapInfo = unwindstack::MapInfo::Create(
      kTargetMapsStart, kTargetMapsEnd, 0, PROT_EXEC | PROT_READ, kTargetName);

  static constexpr uint64_t kTargetAddress1 = 100;
  static constexpr uint64_t kTargetAddress2 = 200;
  static constexpr uint64_t kTargetAddress3 = 300;

  const std::string kFunctionName1 = "foo";
  const std::string kFunctionName2 = "bar";
  const std::string kFunctionName3 = "baz";

  const unwindstack::FrameData kFrame1{
      .pc = kTargetAddress1,
      .function_name = kFunctionName1,
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  const unwindstack::FrameData kFrame2{
      .pc = kTargetAddress2,
      .function_name = kFunctionName2,
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  const unwindstack::FrameData kFrame3{
      .pc = kTargetAddress3,
      .function_name = kFunctionName3,
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  static constexpr uint64_t kNonExecutableMapsStart = 500;
  static constexpr uint64_t kNonExecutableMapsEnd = 600;
  const std::string kNonExecutableName = "data";

  const std::shared_ptr<unwindstack::MapInfo> kNonExecutableMapInfo = unwindstack::MapInfo::Create(
      kNonExecutableMapsStart, kNonExecutableMapsEnd, 0, PROT_EXEC | PROT_READ, kNonExecutableName);

  static constexpr uint64_t kNumOfSpRegisters =
      sizeof(perf_event_sample_regs_user_sp) / sizeof(uint64_t);
};

template <typename>
class UprobesUnwindingVisitorDwarfUnwindingTest
    : public UprobesUnwindingVisitorDwarfUnwindingTestBase {};

template <typename PerfEventType>
PerfEventType BuildFakePerfEventWithStack() {
  constexpr uint64_t kTotalNumOfRegisters =
      sizeof(perf_event_sample_regs_user_all) / sizeof(uint64_t);

  constexpr uint64_t kStackSize = 13;
  PerfEventType result{
      .timestamp = 15,
      .data =
          {
              .regs = std::make_unique<uint64_t[]>(kTotalNumOfRegisters),
              .dyn_size = kStackSize,
              .data = std::make_unique<uint8_t[]>(kStackSize),
          },
  };

  if constexpr (std::is_same_v<StackSamplePerfEvent, PerfEventType>) {
    result.data.pid = 10;
    result.data.tid = 11;
  } else if constexpr (std::is_same_v<SchedWakeupWithStackPerfEvent, PerfEventType>) {
    result.data.was_unblocked_by_pid = 10;
    result.data.was_unblocked_by_tid = 11;
  } else if constexpr (std::is_same_v<SchedSwitchWithStackPerfEvent, PerfEventType>) {
    result.data.prev_pid_or_minus_one = 10;
    result.data.prev_tid = 11;
  }

  return result;
}

TYPED_TEST_SUITE_P(UprobesUnwindingVisitorDwarfUnwindingTest);

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitValidStackSampleWithoutUprobesSendsCompleteCallstackAndAddressInfos) {
  auto event = BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{
      TestFixture::kFrame1, TestFixture::kFrame2, TestFixture::kFrame3};

  std::vector<StackSliceView> actual_stack_slices{};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(
          ::testing::DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                           ::testing::Return(LibunwindstackResult{
                               libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE})));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(3)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  uint64_t dyn_size = event.data.dyn_size;
  uint64_t sp = event.data.GetRegisters().sp;
  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(actual_stack_slices,
              ::testing::ElementsAre(::testing::AllOf(
                  ::testing::Property(&StackSliceView::start_address, ::testing::Eq(sp)),
                  ::testing::Property(&StackSliceView::size, ::testing::Eq(dyn_size)),
                  ::testing::Property(&StackSliceView::data, ::testing::NotNull()))));

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2,
                                     TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitTwoValidStackSamplesSendsAddressInfosOnlyOnce) {
  typename TypeParam::PerfEventT event1 =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();
  typename TypeParam::PerfEventT event2 =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample)
      .Times(2)
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(2).WillRepeatedly(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{
      TestFixture::kFrame1, TestFixture::kFrame2, TestFixture::kFrame3};

  EXPECT_CALL(this->unwinder_, Unwind(event1.data.GetCallstackPidOrMinusOne(), nullptr,
                                      ::testing::_, ::testing::_, ::testing::_, ::testing::_))
      .Times(2)
      .WillRepeatedly(::testing::Return(
          LibunwindstackResult{libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  std::vector<typename TypeParam::ProducerCaptureEventT> actual_callstack_samples;
  auto save_callstack = [&actual_callstack_samples](
                            typename TypeParam::ProducerCaptureEventT actual_callstack_sample) {
    actual_callstack_samples.push_back(std::move(actual_callstack_sample));
  };
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(2)
        .WillRepeatedly(::testing::Invoke(save_callstack));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(2)
        .WillRepeatedly(::testing::Invoke(save_callstack));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(3)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event1)}.Accept(&this->visitor_);
  PerfEvent{std::move(event2)}.Accept(&this->visitor_);

  EXPECT_EQ(actual_callstack_samples.size(), 2);
  EXPECT_THAT(actual_callstack_samples[0].callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2,
                                     TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_samples[0].callstack().type(),
            orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(actual_callstack_samples[1].callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2,
                                     TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_samples[1].callstack().type(),
            orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(
    UprobesUnwindingVisitorDwarfUnwindingTest,
    VisitValidStackSampleWithNullptrMapInfosSendsCompleteCallstackAndAddressInfosWithoutModuleName) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  unwindstack::FrameData frame_1_no_map_info = TestFixture::kFrame1;
  frame_1_no_map_info.map_info = nullptr;
  unwindstack::FrameData frame_2_no_map_info = TestFixture::kFrame2;
  frame_2_no_map_info.map_info = nullptr;
  unwindstack::FrameData frame_3_no_map_info = TestFixture::kFrame3;
  frame_3_no_map_info.map_info = nullptr;
  std::vector<unwindstack::FrameData> libunwindstack_callstack{
      frame_1_no_map_info, frame_2_no_map_info, frame_3_no_map_info};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(3)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2,
                                     TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name, "")),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name, "")),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name, ""))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitEmptyStackSampleWithoutUprobesDoesNothing) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> empty_callstack;

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{empty_callstack, {}, unwindstack::ErrorCode::ERROR_MEMORY_INVALID}));

  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample).Times(0);
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack).Times(0);
  }

  EXPECT_CALL(this->listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitInvalidStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfos) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample)
      .Times(1)
      .WillRepeatedly(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{TestFixture::kFrame1,
                                                               TestFixture::kFrame2};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(LibunwindstackResult{
          libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_MEMORY_INVALID}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(2)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kDwarfUnwindingError);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitSingleFrameStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfos) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> incomplete_callstack{TestFixture::kFrame1};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{incomplete_callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(1)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kDwarfUnwindingError);
  EXPECT_THAT(actual_address_infos,
              ::testing::UnorderedElementsAre(::testing::AllOf(
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                      TestFixture::kTargetAddress1),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                      TestFixture::kFunctionName1),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                      TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitSingleFrameStackSampleInFunctionToStopAtSendsCompleteCallstackAndAddressInfos) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  this->absolute_address_to_size_of_functions_to_stop_at_[TestFixture::kTargetAddress1] = 100;

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{TestFixture::kFrame1};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(1)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(actual_address_infos,
              ::testing::UnorderedElementsAre(::testing::AllOf(
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                      TestFixture::kTargetAddress1),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                      TestFixture::kFunctionName1),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                      TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(
    UprobesUnwindingVisitorDwarfUnwindingTest,
    VisitSingleFrameStackSampleOutsideOfAnyFunctionToStopAtSendsUnwindingErrorAndAddressInfos) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  this->absolute_address_to_size_of_functions_to_stop_at_[TestFixture::kTargetAddress2] = 100;

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{TestFixture::kFrame1};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(1)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kDwarfUnwindingError);
  EXPECT_THAT(actual_address_infos,
              ::testing::UnorderedElementsAre(::testing::AllOf(
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                      TestFixture::kTargetAddress1),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                      TestFixture::kFunctionName1),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                  ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                      TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitStackSampleWithinUprobeSendsInUprobesCallstack) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{TestFixture::kUprobesFrame2, TestFixture::kFrame2};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(2)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(
      actual_callstack_sample.callstack().pcs(),
      ::testing::ElementsAre(TestFixture::kUprobesMapsStart + 1, TestFixture::kTargetAddress2));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kInUprobes);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kUprobesMapsStart + 1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kUprobesName),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kUprobesName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 1);
}

TYPED_TEST_P(
    UprobesUnwindingVisitorDwarfUnwindingTest,
    VisitStackSampleWithinUserSpaceInstrumentationTrampolineSendsInUserSpaceInstrumentationCallstack) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{TestFixture::kEntryTrampolineFrame,
                                                TestFixture::kFrame2};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(2)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(
      actual_callstack_sample.callstack().pcs(),
      ::testing::ElementsAre(TestFixture::kEntryTrampolineAddress, TestFixture::kTargetAddress2));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kEntryTrampolineAddress),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kEntryTrampolineFunctionName),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name, "")),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(
    UprobesUnwindingVisitorDwarfUnwindingTest,
    VisitStackSampleWithinUserSpaceInstrumentationTrampolineAndLibrarySendsInUserSpaceInstrumentationCallstack) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{
      TestFixture::kFrame1, TestFixture::kUserSpaceLibraryFrame, TestFixture::kFrame3,
      TestFixture::kEntryTrampolineFrame};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(4)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  // While this is a Callstack::kInUserSpaceInstrumentation, the innermost frame we used is still
  // one of the "regular" frames in the target, i.e., kFrame1.
  EXPECT_THAT(
      actual_callstack_sample.callstack().pcs(),
      ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kUserSpaceLibraryAddress,
                             TestFixture::kTargetAddress3, TestFixture::kEntryTrampolineAddress));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kUserSpaceLibraryAddress),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kUserSpaceLibraryFunctionName),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kUserSpaceLibraryName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kEntryTrampolineAddress),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kEntryTrampolineFunctionName),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name, ""))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(
    UprobesUnwindingVisitorDwarfUnwindingTest,
    VisitStackSampleWithinUserSpaceInstrumentationLibraryButNotTrampolineSendsCompleteCallstack) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{
      TestFixture::kFrame1, TestFixture::kUserSpaceLibraryFrame, TestFixture::kFrame3};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(3)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(
      actual_callstack_sample.callstack().pcs(),
      ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kUserSpaceLibraryAddress,
                             TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kUserSpaceLibraryAddress),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kUserSpaceLibraryFunctionName),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kUserSpaceLibraryName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitStackSampleStoppedAtUprobesSendsPatchingFailedCallstack) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{TestFixture::kFrame1, TestFixture::kUprobesFrame1};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(2)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kUprobesMapsStart));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kCallstackPatchingFailed);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kUprobesMapsStart),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kUprobesName),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kUprobesName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(
    UprobesUnwindingVisitorDwarfUnwindingTest,
    VisitStackSampleStoppedAtUserSpaceInstrumentationTrampolineSendsPatchingFailedCallstack) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{TestFixture::kFrame1,
                                                TestFixture::kReturnTrampolineFrame};

  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(::testing::Return(
          LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(2)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(
      actual_callstack_sample.callstack().pcs(),
      ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kReturnTrampolineAddress));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kCallstackPatchingFailed);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kReturnTrampolineAddress),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kReturnTrampolineFunctionName),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name, ""))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest, VisitStackSampleUsesUserSpaceStack) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{
      TestFixture::kFrame1, TestFixture::kFrame2, TestFixture::kFrame3};

  std::vector<StackSliceView> actual_stack_slices{};
  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(
          ::testing::DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                           ::testing::Return(LibunwindstackResult{
                               libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE})));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(3)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  constexpr uint64_t kUserStackSize = 1024;
  constexpr uint64_t kUserStackPointer = 16;
  UprobesWithStackPerfEvent user_stack_event{
      .timestamp = 10,
      .data =
          {
              .stream_id = 1,
              .pid = event.data.GetCallstackPidOrMinusOne(),
              .tid = event.data.GetCallstackTid(),
              .regs = std::make_unique<uint64_t[]>(TestFixture::kNumOfSpRegisters),
              .dyn_size = kUserStackSize,
              .data = std::make_unique<uint8_t[]>(kUserStackSize),
          },
  };
  perf_event_sample_regs_user_sp sp_regs{};
  sp_regs.sp = kUserStackPointer;
  std::memcpy(user_stack_event.data.regs.get(), &sp_regs, sizeof(sp_regs));
  uint8_t* user_stack_data = user_stack_event.data.data.get();
  PerfEvent{std::move(user_stack_event)}.Accept(&this->visitor_);

  uint64_t dyn_size = event.data.dyn_size;
  uint64_t sp = event.data.GetRegisters().sp;
  uint8_t* stack_data = event.data.data.get();
  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(
      actual_stack_slices,
      ::testing::ElementsAre(
          ::testing::AllOf(::testing::Property(&StackSliceView::start_address, ::testing::Eq(sp)),
                           ::testing::Property(&StackSliceView::size, ::testing::Eq(dyn_size)),
                           ::testing::Property(&StackSliceView::data, ::testing::Eq(stack_data))),
          ::testing::AllOf(
              ::testing::Property(&StackSliceView::start_address, ::testing::Eq(kUserStackPointer)),
              ::testing::Property(&StackSliceView::size, ::testing::Eq(kUserStackSize)),
              ::testing::Property(&StackSliceView::data, ::testing::Eq(user_stack_data)))));

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2,
                                     TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitStackSampleUsesLatestUserSpaceCallstack) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{
      TestFixture::kFrame1, TestFixture::kFrame2, TestFixture::kFrame3};

  std::vector<StackSliceView> actual_stack_slices{};
  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(
          ::testing::DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                           ::testing::Return(LibunwindstackResult{
                               libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE})));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(3)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  constexpr uint64_t kUserStackSizeOld = 512;
  constexpr uint64_t kUserStackPointerOld = 24;
  UprobesWithStackPerfEvent user_stack_event_old{
      .timestamp = 12,
      .data =
          {
              .stream_id = 1,
              .pid = event.data.GetCallstackPidOrMinusOne(),
              .tid = event.data.GetCallstackTid(),
              .regs = std::make_unique<uint64_t[]>(TestFixture::kNumOfSpRegisters),
              .dyn_size = kUserStackSizeOld,
              .data = std::make_unique<uint8_t[]>(kUserStackSizeOld),
          },
  };
  perf_event_sample_regs_user_sp sp_regs1{};
  sp_regs1.sp = kUserStackPointerOld;
  std::memcpy(user_stack_event_old.data.regs.get(), &sp_regs1, sizeof(sp_regs1));
  PerfEvent{std::move(user_stack_event_old)}.Accept(&this->visitor_);

  constexpr uint64_t kUserStackSizeNew = 1024;
  constexpr uint64_t kUserStackPointerNew = 16;
  UprobesWithStackPerfEvent user_stack_event_new{
      .timestamp = 13,
      .data =
          {
              .stream_id = 1,
              .pid = event.data.GetCallstackPidOrMinusOne(),
              .tid = event.data.GetCallstackTid(),
              .regs = std::make_unique<uint64_t[]>(TestFixture::kNumOfSpRegisters),
              .dyn_size = kUserStackSizeNew,
              .data = std::make_unique<uint8_t[]>(kUserStackSizeNew),
          },
  };
  perf_event_sample_regs_user_sp sp_regs2{};
  sp_regs2.sp = kUserStackPointerNew;
  std::memcpy(user_stack_event_new.data.regs.get(), &sp_regs2, sizeof(sp_regs2));
  uint8_t* user_stack_data = user_stack_event_new.data.data.get();
  PerfEvent{std::move(user_stack_event_new)}.Accept(&this->visitor_);

  uint64_t dyn_size = event.data.dyn_size;
  uint64_t sp = event.data.GetRegisters().sp;
  uint8_t* stack_data = event.data.data.get();
  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(
      actual_stack_slices,
      ::testing::ElementsAre(
          ::testing::AllOf(::testing::Property(&StackSliceView::start_address, ::testing::Eq(sp)),
                           ::testing::Property(&StackSliceView::size, ::testing::Eq(dyn_size)),
                           ::testing::Property(&StackSliceView::data, ::testing::Eq(stack_data))),
          ::testing::AllOf(
              ::testing::Property(&StackSliceView::start_address,
                                  ::testing::Eq(kUserStackPointerNew)),
              ::testing::Property(&StackSliceView::size, ::testing::Eq(kUserStackSizeNew)),
              ::testing::Property(&StackSliceView::data, ::testing::Eq(user_stack_data)))));

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2,
                                     TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitStackSampleUsesUserSpaceCallstackOnlyFromSameThread) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{
      TestFixture::kFrame1, TestFixture::kFrame2, TestFixture::kFrame3};

  std::vector<StackSliceView> actual_stack_slices{};
  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(
          ::testing::DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                           ::testing::Return(LibunwindstackResult{
                               libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE})));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(3)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  constexpr uint64_t kUserStackSizeSameThread = 512;
  constexpr uint64_t kUserStackPointerSameThread = 24;
  UprobesWithStackPerfEvent user_stack_event_same_thread{
      .timestamp = 12,
      .data =
          {
              .stream_id = 1,
              .pid = event.data.GetCallstackPidOrMinusOne(),
              .tid = event.data.GetCallstackTid(),
              .regs = std::make_unique<uint64_t[]>(TestFixture::kNumOfSpRegisters),
              .dyn_size = kUserStackSizeSameThread,
              .data = std::make_unique<uint8_t[]>(kUserStackSizeSameThread),
          },
  };
  perf_event_sample_regs_user_sp sp_regs1{};
  sp_regs1.sp = kUserStackPointerSameThread;
  std::memcpy(user_stack_event_same_thread.data.regs.get(), &sp_regs1, sizeof(sp_regs1));
  uint8_t* user_stack_data = user_stack_event_same_thread.data.data.get();
  PerfEvent{std::move(user_stack_event_same_thread)}.Accept(&this->visitor_);

  constexpr uint64_t kUserStackSizeOtherThread = 1024;
  constexpr uint64_t kUserStackPointerOtherThread = 16;
  UprobesWithStackPerfEvent user_stack_event_other_thread{
      .timestamp = 13,
      .data =
          {
              .stream_id = 1,
              .pid = event.data.GetCallstackPidOrMinusOne(),
              .tid = event.data.GetCallstackTid() + 1,
              .regs = std::make_unique<uint64_t[]>(TestFixture::kNumOfSpRegisters),
              .dyn_size = kUserStackSizeOtherThread,
              .data = std::make_unique<uint8_t[]>(kUserStackSizeOtherThread),
          },
  };
  perf_event_sample_regs_user_sp sp_regs2{};
  sp_regs2.sp = kUserStackPointerOtherThread;
  std::memcpy(user_stack_event_other_thread.data.regs.get(), &sp_regs2, sizeof(sp_regs2));
  PerfEvent{std::move(user_stack_event_other_thread)}.Accept(&this->visitor_);

  uint64_t dyn_size = event.data.dyn_size;
  uint64_t sp = event.data.GetRegisters().sp;
  uint8_t* stack_data = event.data.data.get();
  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  EXPECT_THAT(
      actual_stack_slices,
      ::testing::ElementsAre(
          ::testing::AllOf(::testing::Property(&StackSliceView::start_address, ::testing::Eq(sp)),
                           ::testing::Property(&StackSliceView::size, ::testing::Eq(dyn_size)),
                           ::testing::Property(&StackSliceView::data, ::testing::Eq(stack_data))),
          ::testing::AllOf(
              ::testing::Property(&StackSliceView::start_address,
                                  ::testing::Eq(kUserStackPointerSameThread)),
              ::testing::Property(&StackSliceView::size, ::testing::Eq(kUserStackSizeSameThread)),
              ::testing::Property(&StackSliceView::data, ::testing::Eq(user_stack_data)))));

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2,
                                     TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TYPED_TEST_P(UprobesUnwindingVisitorDwarfUnwindingTest,
             VisitStackSampleUsesUserStackMemoryFromAllStreamIds) {
  typename TypeParam::PerfEventT event =
      BuildFakePerfEventWithStack<typename TypeParam::PerfEventT>();

  EXPECT_CALL(this->return_address_manager_, PatchSample).Times(1).WillOnce(::testing::Return());
  EXPECT_CALL(this->maps_, Get).Times(1).WillOnce(::testing::Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{
      TestFixture::kFrame1, TestFixture::kFrame2, TestFixture::kFrame3};

  std::vector<StackSliceView> actual_stack_slices{};
  EXPECT_CALL(this->unwinder_, Unwind(event.data.GetCallstackPidOrMinusOne(), nullptr, ::testing::_,
                                      ::testing::_, ::testing::_, ::testing::_))
      .Times(1)
      .WillOnce(
          ::testing::DoAll(SaveRangeFromArg<3>(&actual_stack_slices),
                           ::testing::Return(LibunwindstackResult{
                               libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE})));

  typename TypeParam::ProducerCaptureEventT actual_callstack_sample;
  if constexpr (std::is_same_v<StackSamplePerfEvent, typename TypeParam::PerfEventT>) {
    EXPECT_CALL(this->listener_, OnCallstackSample)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  } else {
    EXPECT_CALL(this->listener_, OnThreadStateSliceCallstack)
        .Times(1)
        .WillOnce(::testing::SaveArg<0>(&actual_callstack_sample));
  }

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(this->listener_, OnAddressInfo)
      .Times(3)
      .WillRepeatedly(::testing::Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  this->visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(
      &unwinding_errors, &discarded_samples_in_uretprobes_counter);

  constexpr uint64_t kUserStackSize1 = 512;
  constexpr uint64_t kUserStackPointer1 = 24;
  UprobesWithStackPerfEvent user_stack_event1{
      .timestamp = 12,
      .data =
          {
              .stream_id = 1,
              .pid = event.data.GetCallstackPidOrMinusOne(),
              .tid = event.data.GetCallstackTid(),
              .regs = std::make_unique<uint64_t[]>(TestFixture::kNumOfSpRegisters),
              .dyn_size = kUserStackSize1,
              .data = std::make_unique<uint8_t[]>(kUserStackSize1),
          },
  };
  perf_event_sample_regs_user_sp sp_regs1{};
  sp_regs1.sp = kUserStackPointer1;
  std::memcpy(user_stack_event1.data.regs.get(), &sp_regs1, sizeof(sp_regs1));
  uint8_t* user_stack_data1 = user_stack_event1.data.data.get();
  PerfEvent{std::move(user_stack_event1)}.Accept(&this->visitor_);

  constexpr uint64_t kUserStackSize2 = 1024;
  constexpr uint64_t kUserStackPointer2 = 16;
  UprobesWithStackPerfEvent user_stack_event2{
      .timestamp = 13,
      .data =
          {
              .stream_id = 2,
              .pid = event.data.GetCallstackPidOrMinusOne(),
              .tid = event.data.GetCallstackTid(),
              .regs = std::make_unique<uint64_t[]>(TestFixture::kNumOfSpRegisters),
              .dyn_size = kUserStackSize2,
              .data = std::make_unique<uint8_t[]>(kUserStackSize2),
          },
  };
  perf_event_sample_regs_user_sp sp_regs2{};
  sp_regs2.sp = kUserStackPointer2;
  std::memcpy(user_stack_event2.data.regs.get(), &sp_regs2, sizeof(sp_regs2));
  uint8_t* user_stack_data2 = user_stack_event2.data.data.get();
  PerfEvent{std::move(user_stack_event2)}.Accept(&this->visitor_);

  uint64_t dyn_size = event.data.dyn_size;
  uint64_t sp = event.data.GetRegisters().sp;
  uint8_t* stack_data = event.data.data.get();
  PerfEvent{std::move(event)}.Accept(&this->visitor_);

  // We don't guarantee an order for the stack slices of different stream ids. However, the first
  // element must be the stack slice from the sample.
  ASSERT_THAT(
      actual_stack_slices,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(::testing::Property(&StackSliceView::start_address, ::testing::Eq(sp)),
                           ::testing::Property(&StackSliceView::size, ::testing::Eq(dyn_size)),
                           ::testing::Property(&StackSliceView::data, ::testing::Eq(stack_data))),
          ::testing::AllOf(
              ::testing::Property(&StackSliceView::start_address,
                                  ::testing::Eq(kUserStackPointer1)),
              ::testing::Property(&StackSliceView::size, ::testing::Eq(kUserStackSize1)),
              ::testing::Property(&StackSliceView::data, ::testing::Eq(user_stack_data1))),
          ::testing::AllOf(
              ::testing::Property(&StackSliceView::start_address,
                                  ::testing::Eq(kUserStackPointer2)),
              ::testing::Property(&StackSliceView::size, ::testing::Eq(kUserStackSize2)),
              ::testing::Property(&StackSliceView::data, ::testing::Eq(user_stack_data2)))));
  EXPECT_THAT(
      actual_stack_slices[0],
      ::testing::AllOf(::testing::Property(&StackSliceView::start_address, ::testing::Eq(sp)),
                       ::testing::Property(&StackSliceView::size, ::testing::Eq(dyn_size)),
                       ::testing::Property(&StackSliceView::data, ::testing::Eq(stack_data))));

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ::testing::ElementsAre(TestFixture::kTargetAddress1, TestFixture::kTargetAddress2,
                                     TestFixture::kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      ::testing::UnorderedElementsAre(
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName1),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName2),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName)),
          ::testing::AllOf(
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                                  TestFixture::kTargetAddress3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                                  TestFixture::kFunctionName3),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
              ::testing::Property(&orbit_grpc_protos::FullAddressInfo::module_name,
                                  TestFixture::kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

REGISTER_TYPED_TEST_SUITE_P(
    UprobesUnwindingVisitorDwarfUnwindingTest,
    VisitValidStackSampleWithoutUprobesSendsCompleteCallstackAndAddressInfos,
    VisitEmptyStackSampleWithoutUprobesDoesNothing,
    VisitInvalidStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfos,
    VisitSingleFrameStackSampleInFunctionToStopAtSendsCompleteCallstackAndAddressInfos,
    VisitSingleFrameStackSampleOutsideOfAnyFunctionToStopAtSendsUnwindingErrorAndAddressInfos,
    VisitSingleFrameStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfos,
    VisitStackSampleStoppedAtUprobesSendsPatchingFailedCallstack,
    VisitStackSampleStoppedAtUserSpaceInstrumentationTrampolineSendsPatchingFailedCallstack,
    VisitStackSampleUsesLatestUserSpaceCallstack,
    VisitStackSampleUsesUserSpaceCallstackOnlyFromSameThread, VisitStackSampleUsesUserSpaceStack,
    VisitStackSampleUsesUserStackMemoryFromAllStreamIds,
    VisitStackSampleWithinUprobeSendsInUprobesCallstack,
    VisitStackSampleWithinUserSpaceInstrumentationLibraryButNotTrampolineSendsCompleteCallstack,
    VisitStackSampleWithinUserSpaceInstrumentationTrampolineAndLibrarySendsInUserSpaceInstrumentationCallstack,
    VisitStackSampleWithinUserSpaceInstrumentationTrampolineSendsInUserSpaceInstrumentationCallstack,
    VisitTwoValidStackSamplesSendsAddressInfosOnlyOnce,
    VisitValidStackSampleWithNullptrMapInfosSendsCompleteCallstackAndAddressInfosWithoutModuleName);

template <typename T, typename U>
struct DwarfUnwindingTestType {
  using PerfEventT = T;
  using ProducerCaptureEventT = U;
};

using TestTypes = ::testing::Types<
    DwarfUnwindingTestType<StackSamplePerfEvent, orbit_grpc_protos::FullCallstackSample>,
    DwarfUnwindingTestType<SchedWakeupWithStackPerfEvent,
                           orbit_grpc_protos::ThreadStateSliceCallstack>,
    DwarfUnwindingTestType<SchedSwitchWithStackPerfEvent,
                           orbit_grpc_protos::ThreadStateSliceCallstack>>;
INSTANTIATE_TYPED_TEST_SUITE_P(TypedTest, UprobesUnwindingVisitorDwarfUnwindingTest, TestTypes);
}  // namespace orbit_linux_tracing
