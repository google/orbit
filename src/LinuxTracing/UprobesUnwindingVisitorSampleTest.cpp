// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>
#include <unwindstack/Error.h>
#include <unwindstack/MapInfo.h>
#include <unwindstack/Unwinder.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "MockTracerListener.h"
#include "OrbitBase/Logging.h"
#include "PerfEvent.h"
#include "PerfEventRecords.h"
#include "Test/Path.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesReturnAddressManager.h"
#include "UprobesUnwindingVisitor.h"
#include "UprobesUnwindingVisitorTestCommon.h"

using ::testing::_;
using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ge;
using ::testing::Invoke;
using ::testing::Lt;
using ::testing::NotNull;
using ::testing::Property;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::UnorderedElementsAre;

using orbit_grpc_protos::Callstack;

namespace orbit_linux_tracing {

namespace {

class UprobesUnwindingVisitorSampleTest : public ::testing::Test {
 protected:
  void SetUp() override {
    EXPECT_CALL(maps_, Find(AllOf(Ge(kUprobesMapsStart), Lt(kUprobesMapsEnd))))
        .WillRepeatedly(Return(kUprobesMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kTargetMapsStart), Lt(kTargetMapsEnd))))
        .WillRepeatedly(Return(kTargetMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kNonExecutableMapsStart), Lt(kNonExecutableMapsEnd))))
        .WillRepeatedly(Return(kNonExecutableMapInfo));
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
  static inline const std::shared_ptr<unwindstack::MapInfo> kUserSpaceLibraryMapInfo =
      unwindstack::MapInfo::Create(kUserSpaceLibraryMapsStart, kUserSpaceLibraryMapsEnd, 0,
                                   PROT_EXEC | PROT_READ, kUserSpaceLibraryName);

  static constexpr uint64_t kUserSpaceLibraryAddress = kUserSpaceLibraryMapsStart;
  static inline const std::string kUserSpaceLibraryFunctionName = "payload";
  static inline const unwindstack::FrameData kUserSpaceLibraryFrame{
      .pc = kUserSpaceLibraryAddress,
      .function_name = kUserSpaceLibraryFunctionName,
      .function_offset = 0,
      .map_info = kUserSpaceLibraryMapInfo,
  };

  static constexpr uint64_t kEntryTrampolineAddress = 0xAAAAAAAAAAAAAA00LU;
  static inline const std::string kEntryTrampolineFunctionName = "entry_trampoline";
  static inline const std::shared_ptr<unwindstack::MapInfo> kEntryTrampolineMapInfo =
      unwindstack::MapInfo::Create(kEntryTrampolineAddress, kEntryTrampolineAddress + 0x1000, 0,
                                   PROT_EXEC | PROT_READ, "");
  static inline const unwindstack::FrameData kEntryTrampolineFrame{
      .pc = kEntryTrampolineAddress,
      .function_name = kEntryTrampolineFunctionName,
      .function_offset = 0,
      .map_info = kEntryTrampolineMapInfo,
  };

  static constexpr uint64_t kReturnTrampolineAddress = 0xBBBBBBBBBBBBBB00LU;
  static inline const std::string kReturnTrampolineFunctionName = "return_trampoline";
  static inline const std::shared_ptr<unwindstack::MapInfo> kReturnTrampolineMapInfo =
      unwindstack::MapInfo::Create(kReturnTrampolineAddress, kReturnTrampolineAddress + 0x1000, 0,
                                   PROT_EXEC | PROT_READ, "");
  static inline const unwindstack::FrameData kReturnTrampolineFrame{
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

  static constexpr uint64_t kKernelAddress = 0xFFFFFFFFFFFFFE00;

  static inline const std::string kUprobesName = "[uprobes]";
  static constexpr uint64_t kUprobesMapsStart = 0x7FFFFFFFE000;
  static constexpr uint64_t kUprobesMapsEnd = 0x7FFFFFFFE001;
  static inline const std::shared_ptr<unwindstack::MapInfo> kUprobesMapInfo =
      unwindstack::MapInfo::Create(kUprobesMapsStart, kUprobesMapsEnd, 0, PROT_EXEC | PROT_READ,
                                   kUprobesName);

  static inline const unwindstack::FrameData kUprobesFrame1{
      .pc = kUprobesMapsStart,
      .function_name = "uprobe",
      .function_offset = 0,
      .map_info = kUprobesMapInfo,
  };

  static inline const unwindstack::FrameData kUprobesFrame2{
      .pc = kUprobesMapsStart + 1,
      .function_name = "uprobe",
      .function_offset = 0,
      .map_info = kUprobesMapInfo,
  };

  static inline const std::string kTargetName = "target";
  static constexpr uint64_t kTargetMapsStart = 100;
  static constexpr uint64_t kTargetMapsEnd = 400;
  static inline const std::shared_ptr<unwindstack::MapInfo> kTargetMapInfo =
      unwindstack::MapInfo::Create(kTargetMapsStart, kTargetMapsEnd, 0, PROT_EXEC | PROT_READ,
                                   kTargetName);

  static constexpr uint64_t kTargetAddress1 = 100;
  static constexpr uint64_t kTargetAddress2 = 200;
  static constexpr uint64_t kTargetAddress3 = 300;

  static inline const std::string kFunctionName1 = "foo";
  static inline const std::string kFunctionName2 = "bar";
  static inline const std::string kFunctionName3 = "baz";

  static inline const unwindstack::FrameData kFrame1{
      .pc = kTargetAddress1,
      .function_name = kFunctionName1,
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  static inline const unwindstack::FrameData kFrame2{
      .pc = kTargetAddress2,
      .function_name = kFunctionName2,
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  static inline const unwindstack::FrameData kFrame3{
      .pc = kTargetAddress3,
      .function_name = kFunctionName3,
      .function_offset = 0,
      .map_info = kTargetMapInfo,
  };

  static constexpr uint64_t kNonExecutableMapsStart = 500;
  static constexpr uint64_t kNonExecutableMapsEnd = 600;
  static inline const std::string kNonExecutableName = "data";

  static inline const std::shared_ptr<unwindstack::MapInfo> kNonExecutableMapInfo =
      unwindstack::MapInfo::Create(kNonExecutableMapsStart, kNonExecutableMapsEnd, 0,
                                   PROT_EXEC | PROT_READ, kNonExecutableName);
};

StackSamplePerfEvent BuildFakeStackSamplePerfEvent() {
  constexpr uint64_t kStackSize = 13;
  return StackSamplePerfEvent{
      .timestamp = 15,
      .data =
          {
              .pid = 10,
              .tid = 11,
              .regs = std::make_unique<perf_event_sample_regs_user_all>(),
              .dyn_size = kStackSize,
              .data = std::make_unique<char[]>(kStackSize),
          },
  };
}

CallchainSamplePerfEvent BuildFakeCallchainSamplePerfEvent(const std::vector<uint64_t>& callchain) {
  constexpr uint64_t kStackSize = 13;
  CallchainSamplePerfEvent event{
      .timestamp = 15,
      .data =
          {
              .pid = 10,
              .tid = 11,
              .regs = std::make_unique<perf_event_sample_regs_user_all>(),
              .data = std::make_unique<char[]>(kStackSize),
          },
  };
  event.data.SetIps(callchain);
  return event;
}

}  // namespace

//--------------------------------//
// Visit StackSamplePerfEventData //
//--------------------------------//

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitValidStackSampleWithoutUprobesSendsCompleteCallstackAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{kFrame1, kFrame2, kFrame3};

  std::vector<StackSliceView> actual_stack_slices{};
  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(DoAll(SaveArg<3>(&actual_stack_slices),
                      Return(LibunwindstackResult{
                          libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE})));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(3).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  uint64_t dyn_size = event.data.dyn_size;
  uint64_t sp = event.data.regs->sp;
  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_stack_slices,
              ElementsAre(AllOf(Property(&StackSliceView::start_address, Eq(sp)),
                                Property(&StackSliceView::size, Eq(dyn_size)),
                                Property(&StackSliceView::data, NotNull()))));

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress2),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName2),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress3),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName3),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest, VisitTwoValidStackSamplesSendsAddressInfosOnlyOnce) {
  StackSamplePerfEvent event1 = BuildFakeStackSamplePerfEvent();
  StackSamplePerfEvent event2 = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(2).WillRepeatedly(Return());
  EXPECT_CALL(maps_, Get).Times(2).WillRepeatedly(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{kFrame1, kFrame2, kFrame3};

  EXPECT_CALL(unwinder_, Unwind(event1.data.pid, nullptr, _, _, _, _))
      .Times(2)
      .WillRepeatedly(Return(
          LibunwindstackResult{libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  std::vector<orbit_grpc_protos::FullCallstackSample> actual_callstack_samples;
  auto save_callstack =
      [&actual_callstack_samples](orbit_grpc_protos::FullCallstackSample actual_callstack_sample) {
        actual_callstack_samples.push_back(std::move(actual_callstack_sample));
      };
  EXPECT_CALL(listener_, OnCallstackSample).Times(2).WillRepeatedly(Invoke(save_callstack));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(3).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event1)}.Accept(&visitor_);
  PerfEvent{std::move(event2)}.Accept(&visitor_);

  EXPECT_EQ(actual_callstack_samples.size(), 2);
  EXPECT_THAT(actual_callstack_samples[0].callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_samples[0].callstack().type(),
            orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(actual_callstack_samples[1].callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_samples[1].callstack().type(),
            orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress2),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName2),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress3),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName3),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorSampleTest,
    VisitValidStackSampleWithNullptrMapInfosSendsCompleteCallstackAndAddressInfosWithoutModuleName) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  unwindstack::FrameData frame_1_no_map_info = kFrame1;
  frame_1_no_map_info.map_info = nullptr;
  unwindstack::FrameData frame_2_no_map_info = kFrame2;
  frame_2_no_map_info.map_info = nullptr;
  unwindstack::FrameData frame_3_no_map_info = kFrame3;
  frame_3_no_map_info.map_info = nullptr;
  std::vector<unwindstack::FrameData> libunwindstack_callstack{
      frame_1_no_map_info, frame_2_no_map_info, frame_3_no_map_info};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(
          LibunwindstackResult{libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(3).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, "")),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress2),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName2),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, "")),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress3),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName3),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, ""))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest, VisitEmptyStackSampleWithoutUprobesDoesNothing) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> empty_callstack;

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(
          LibunwindstackResult{empty_callstack, {}, unwindstack::ErrorCode::ERROR_MEMORY_INVALID}));

  EXPECT_CALL(listener_, OnCallstackSample).Times(0);

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitInvalidStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillRepeatedly(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{kFrame1, kFrame2};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{
          libunwindstack_callstack, {}, unwindstack::ErrorCode::ERROR_MEMORY_INVALID}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(2).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kDwarfUnwindingError);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress2),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName2),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitSingleFrameStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> incomplete_callstack{kFrame1};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(
          LibunwindstackResult{incomplete_callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(1).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kDwarfUnwindingError);
  EXPECT_THAT(actual_address_infos,
              UnorderedElementsAre(AllOf(
                  Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                  Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                  Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                  Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitSingleFrameStackSampleInFunctionToStopAtSendsCompleteCallstackAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  absolute_address_to_size_of_functions_to_stop_at_[kTargetAddress1] = 100;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(1).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(actual_address_infos,
              UnorderedElementsAre(AllOf(
                  Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                  Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                  Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                  Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitSingleFrameStackSampleOutsideOfAnyFunctionToStopAtSendsUnwindingErrorAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  absolute_address_to_size_of_functions_to_stop_at_[kTargetAddress2] = 100;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(1).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kDwarfUnwindingError);
  EXPECT_THAT(actual_address_infos,
              UnorderedElementsAre(AllOf(
                  Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                  Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                  Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                  Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest, VisitStackSampleWithinUprobeSendsInUprobesCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kUprobesFrame2, kFrame2};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(2).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kUprobesMapsStart + 1, kTargetAddress2));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kInUprobes);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                         kUprobesMapsStart + 1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kUprobesName),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 1),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kUprobesName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress2),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName2),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 1);
}

TEST_F(
    UprobesUnwindingVisitorSampleTest,
    VisitStackSampleWithinUserSpaceInstrumentationTrampolineSendsInUserSpaceInstrumentationCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kEntryTrampolineFrame, kFrame2};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(2).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kEntryTrampolineAddress, kTargetAddress2));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                         kEntryTrampolineAddress),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                         kEntryTrampolineFunctionName),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, "")),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress2),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName2),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorSampleTest,
    VisitStackSampleWithinUserSpaceInstrumentationTrampolineAndLibrarySendsInUserSpaceInstrumentationCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1, kUserSpaceLibraryFrame, kFrame3,
                                                kEntryTrampolineFrame};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(4).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  // While this is a Callstack::kInUserSpaceInstrumentation, the innermost frame we used is still
  // one of the "regular" frames in the target, i.e., kFrame1.
  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUserSpaceLibraryAddress, kTargetAddress3,
                          kEntryTrampolineAddress));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                         kUserSpaceLibraryAddress),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                         kUserSpaceLibraryFunctionName),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kUserSpaceLibraryName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress3),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName3),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                         kEntryTrampolineAddress),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                         kEntryTrampolineFunctionName),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, ""))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorSampleTest,
    VisitStackSampleWithinUserSpaceInstrumentationLibraryButNotTrampolineSendsCompleteCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1, kUserSpaceLibraryFrame, kFrame3};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(3).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUserSpaceLibraryAddress, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                         kUserSpaceLibraryAddress),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                         kUserSpaceLibraryFunctionName),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kUserSpaceLibraryName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress3),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName3),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitStackSampleStoppedAtUprobesSendsPatchingFailedCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1, kUprobesFrame1};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(2).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUprobesMapsStart));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kCallstackPatchingFailed);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kUprobesMapsStart),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kUprobesName),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kUprobesName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitStackSampleStoppedAtUserSpaceInstrumentationTrampolineSendsPatchingFailedCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1, kReturnTrampolineFrame};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, {}, unwindstack::ErrorCode::ERROR_NONE}));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::vector<orbit_grpc_protos::FullAddressInfo> actual_address_infos;
  auto save_address_info =
      [&actual_address_infos](orbit_grpc_protos::FullAddressInfo actual_address_info) {
        actual_address_infos.push_back(std::move(actual_address_info));
      };
  EXPECT_CALL(listener_, OnAddressInfo).Times(2).WillRepeatedly(Invoke(save_address_info));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kReturnTrampolineAddress));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kCallstackPatchingFailed);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, kFunctionName1),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address,
                         kReturnTrampolineAddress),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name,
                         kReturnTrampolineFunctionName),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, ""))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

//------------------------------------//
// Visit CallchainSamplePerfEventData //
//------------------------------------//

TEST_F(UprobesUnwindingVisitorSampleTest, VisitValidCallchainSampleWithoutUprobesSendsCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kTargetAddress2 + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillOnce(Return(true));
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kComplete));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest, VisitSingleFrameCallchainSampleDoesNothing) {
  std::vector<uint64_t> callchain{kKernelAddress};

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  EXPECT_CALL(listener_, OnCallstackSample).Times(0);

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitCallchainSampleInsideUprobeCodeSendsInUprobesCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kUprobesMapsStart,
      // Increment by one as the return address is the next address.
      kTargetAddress2 + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(maps_, Find(kUprobesMapsStart)).WillRepeatedly(Return(kUprobesMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kUprobesMapsStart, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kInUprobes);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 1);
}

TEST_F(
    UprobesUnwindingVisitorSampleTest,
    VisitCallchainSampleInsideUserSpaceInstrumentationTrampolineSendsInUserSpaceInstrumentationCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kEntryTrampolineAddress,
      // Increment by one as the return address is the next address.
      kTargetAddress2 + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(maps_, Find(kEntryTrampolineAddress)).WillRepeatedly(Return(nullptr));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kEntryTrampolineAddress, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorSampleTest,
    VisitCallchainSampleInsideUserSpaceInstrumentationLibrarySendsInUserSpaceInstrumentationCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kUserSpaceLibraryAddress + 1,
      kTargetAddress3 + 1,
      kEntryTrampolineAddress + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(maps_, Find(AllOf(Ge(kUserSpaceLibraryMapsStart), Lt(kUserSpaceLibraryMapsEnd))))
      .WillRepeatedly(Return(kUserSpaceLibraryMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  // While this is a Callstack::kInUserSpaceInstrumentation, the innermost frame we used is still
  // one of the "regular" frames in the target, i.e., at kTargetAddress1.
  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUserSpaceLibraryAddress, kTargetAddress3,
                          kEntryTrampolineAddress));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorSampleTest,
    VisitCallchainSampleInsideUserSpaceInstrumentationLibraryAfterLeafFunctionPatchingSendsInUserSpaceInstrumentationCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      // `kUserSpaceLibraryAddress + 1` is the missing frame.
      kTargetAddress3 + 1,
      kEntryTrampolineAddress + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(maps_, Find(AllOf(Ge(kUserSpaceLibraryMapsStart), Lt(kUserSpaceLibraryMapsEnd))))
      .WillRepeatedly(Return(kUserSpaceLibraryMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce([](const CallchainSamplePerfEventData* event_data,
                   LibunwindstackMaps* /*current_maps*/, LibunwindstackUnwinder* /*unwinder*/) {
        event_data->SetIps({
            kKernelAddress,
            kTargetAddress1,
            kUserSpaceLibraryAddress + 1,  // This was the missing frame.
            kTargetAddress3 + 1,
            kEntryTrampolineAddress + 1,
        });
        return Callstack::kComplete;
      });

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  // While this is a Callstack::kInUserSpaceInstrumentation, the innermost frame we used is still
  // one of the "regular" frames in the target, i.e., at kTargetAddress1.
  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUserSpaceLibraryAddress, kTargetAddress3,
                          kEntryTrampolineAddress));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kInUserSpaceInstrumentation);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest, VisitPatchableCallchainSampleSendsCompleteCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kUprobesMapsStart + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  auto fake_patch_callchain = [](pid_t /*tid*/, uint64_t* callchain, uint64_t callchain_size,
                                 orbit_linux_tracing::LibunwindstackMaps*) -> bool {
    ORBIT_CHECK(callchain != nullptr);
    ORBIT_CHECK(callchain_size == 4);
    callchain[2] = kTargetAddress2 + 1;
    return true;
  };
  EXPECT_CALL(return_address_manager_, PatchCallchain)
      .Times(1)
      .WillOnce(Invoke(fake_patch_callchain));

  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kComplete));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitUnpatchableCallchainSampleSendsPatchingFailedCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kUprobesMapsStart + 1,
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kComplete));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillOnce(Return(false));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kUprobesMapsStart, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kCallstackPatchingFailed);

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorSampleTest,
       VisitLeafCallOptimizedCallchainSampleWithoutUprobesSendsCompleteCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillRepeatedly(Return(true));

  auto fake_patch_caller_of_leaf_function =
      [](const CallchainSamplePerfEventData* event_data, LibunwindstackMaps* /*maps*/,
         orbit_linux_tracing::LibunwindstackUnwinder*) -> Callstack::CallstackType {
    ORBIT_CHECK(event_data != nullptr);
    std::vector<uint64_t> patched_callchain;
    EXPECT_THAT(event_data->CopyOfIpsAsVector(),
                ElementsAre(kKernelAddress, kTargetAddress1, kTargetAddress3 + 1));
    patched_callchain.push_back(kKernelAddress);
    patched_callchain.push_back(kTargetAddress1);
    // Patch in the missing frame:
    patched_callchain.push_back(kTargetAddress2 + 1);
    patched_callchain.push_back(kTargetAddress3 + 1);
    event_data->SetIps(patched_callchain);
    return Callstack::kComplete;
  };
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Invoke(fake_patch_caller_of_leaf_function));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorSampleTest,
    VisitLeafCallOptimizedCallchainSampleWherePatchingLeafFunctionCallerFailsSendsFramePointerUnwindingErrorCallstack) {
  std::vector<uint64_t> callchain{
      kKernelAddress,
      kTargetAddress1,
      // Increment by one as the return address is the next address.
      kTargetAddress3 + 1,
  };

  CallchainSamplePerfEvent event = BuildFakeCallchainSamplePerfEvent(callchain);

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);

  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kFramePointerUnwindingError));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_.SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                      &discarded_samples_in_uretprobes_counter);

  PerfEvent{std::move(event)}.Accept(&visitor_);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kFramePointerUnwindingError);

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

}  // namespace orbit_linux_tracing
