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
#include <atomic>
#include <cstdint>
#include <ctime>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "LeafFunctionCallManager.h"
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

using ::testing::_;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Ge;
using ::testing::Invoke;
using ::testing::Lt;
using ::testing::Mock;
using ::testing::Property;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::UnorderedElementsAre;

using orbit_grpc_protos::Callstack;

namespace orbit_linux_tracing {

namespace {

class MockLibunwindstackMaps : public LibunwindstackMaps {
 public:
  MOCK_METHOD(std::shared_ptr<unwindstack::MapInfo>, Find, (uint64_t), (override));
  MOCK_METHOD(unwindstack::Maps*, Get, (), (override));
  MOCK_METHOD(void, AddAndSort, (uint64_t, uint64_t, uint64_t, uint64_t, const std::string&),
              (override));
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

class MockUprobesReturnAddressManager : public UprobesReturnAddressManager {
 public:
  explicit MockUprobesReturnAddressManager(
      UserSpaceInstrumentationAddresses* user_space_instrumentation_addresses)
      : UprobesReturnAddressManager{user_space_instrumentation_addresses} {}

  MOCK_METHOD(void, ProcessFunctionEntry, (pid_t, uint64_t, uint64_t), (override));
  MOCK_METHOD(void, ProcessFunctionExit, (pid_t), (override));
  MOCK_METHOD(void, PatchSample, (pid_t, uint64_t, void*, uint64_t), (override));
  MOCK_METHOD(bool, PatchCallchain, (pid_t, uint64_t*, uint64_t, LibunwindstackMaps*), (override));
};

class MockLeafFunctionCallManager : public LeafFunctionCallManager {
 public:
  explicit MockLeafFunctionCallManager(uint16_t stack_dump_size)
      : LeafFunctionCallManager(stack_dump_size) {}
  MOCK_METHOD(Callstack::CallstackType, PatchCallerOfLeafFunction,
              (const CallchainSamplePerfEventData*, LibunwindstackMaps*, LibunwindstackUnwinder*),
              (override));
};

class UprobesUnwindingVisitorTest : public ::testing::Test {
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

TEST_F(UprobesUnwindingVisitorTest,
       VisitDynamicInstrumentationPerfEventsInVariousCombinationsSendsFunctionCalls) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  constexpr uint32_t kCpu = 1;

  {
    UprobesPerfEvent uprobe1{
        .timestamp = 100,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .cpu = kCpu,
                .function_id = 1,
                .sp = 0x50,
                .ip = 0x01,
                .return_address = 0x00,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x50, 0x00)).Times(1);
    PerfEvent{uprobe1}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UprobesWithArgumentsPerfEvent uprobe2{
        .timestamp = 200,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .cpu = kCpu,
                .function_id = 2,
                .return_address = 0x01,
                .regs =
                    {
                        .cx = 4,
                        .dx = 3,
                        .si = 2,
                        .di = 1,
                        .sp = 0x40,
                        .ip = 0x02,
                        .r8 = 5,
                        .r9 = 6,
                    },
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x40, 0x01)).Times(1);
    PerfEvent{uprobe2}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UserSpaceFunctionEntryPerfEvent function_entry3{
        .timestamp = 300,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .function_id = 3,
                .sp = 0x30,
                .return_address = 0x02,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x30, 0x02)).Times(1);
    PerfEvent{function_entry3}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UprobesPerfEvent uprobe4{
        .timestamp = 400,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .cpu = kCpu,
                .function_id = 4,
                .sp = 0x20,
                .ip = 0x04,
                .return_address = 0x03,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x20, 0x03)).Times(1);
    PerfEvent{uprobe4}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UprobesWithArgumentsPerfEvent uprobe5{
        .timestamp = 500,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .cpu = kCpu,
                .function_id = 5,
                .return_address = 0x04,
                .regs =
                    {
                        .cx = 4,
                        .dx = 3,
                        .si = 2,
                        .di = 1,
                        .sp = 0x10,
                        .ip = 0x05,
                        .r8 = 5,
                        .r9 = 6,
                    },
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x10, 0x04)).Times(1);
    PerfEvent{uprobe5}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UretprobesWithReturnValuePerfEvent uretprobe5{
        .timestamp = 600,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .rax = 456,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{uretprobe5}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 5);
    EXPECT_EQ(actual_function_call.duration_ns(), 100);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 600);
    EXPECT_EQ(actual_function_call.depth(), 4);
    EXPECT_EQ(actual_function_call.return_value(), 456);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre(1, 2, 3, 4, 5, 6));
  }

  {
    UretprobesWithReturnValuePerfEvent uretprobe4{
        .timestamp = 700,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .rax = 123,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{uretprobe4}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 4);
    EXPECT_EQ(actual_function_call.duration_ns(), 300);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 700);
    EXPECT_EQ(actual_function_call.depth(), 3);
    EXPECT_EQ(actual_function_call.return_value(), 123);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }

  {
    UserSpaceFunctionExitPerfEvent function_exit3{
        .timestamp = 800,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{function_exit3}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 3);
    EXPECT_EQ(actual_function_call.duration_ns(), 500);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 800);
    EXPECT_EQ(actual_function_call.depth(), 2);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }

  {
    UretprobesPerfEvent uretprobe2{
        .timestamp = 900,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{uretprobe2}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 2);
    EXPECT_EQ(actual_function_call.duration_ns(), 700);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 900);
    EXPECT_EQ(actual_function_call.depth(), 1);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre(1, 2, 3, 4, 5, 6));
  }

  {
    UretprobesPerfEvent uretprobe1{
        .timestamp = 1000,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{uretprobe1}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 1);
    EXPECT_EQ(actual_function_call.duration_ns(), 900);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 1000);
    EXPECT_EQ(actual_function_call.depth(), 0);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }
}

//--------------------------------//
// Visit StackSamplePerfEventData //
//--------------------------------//

TEST_F(UprobesUnwindingVisitorTest,
       VisitValidStackSampleWithoutUprobesSendsCompleteCallstackAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{kFrame1, kFrame2, kFrame3};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest, VisitTwoValidStackSamplesSendsAddressInfosOnlyOnce) {
  StackSamplePerfEvent event1 = BuildFakeStackSamplePerfEvent();
  StackSamplePerfEvent event2 = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(2).WillRepeatedly(Return());
  EXPECT_CALL(maps_, Get).Times(2).WillRepeatedly(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{kFrame1, kFrame2, kFrame3};

  EXPECT_CALL(unwinder_, Unwind(event1.data.pid, nullptr, _, _, event1.data.dyn_size, _, _))
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
    UprobesUnwindingVisitorTest,
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

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest, VisitEmptyStackSampleWithoutUprobesDoesNothing) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> empty_callstack;

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest,
       VisitInvalidStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillRepeatedly(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack{kFrame1, kFrame2};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest,
       VisitSingleFrameStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> incomplete_callstack{kFrame1};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest,
       VisitSingleFrameStackSampleInFunctionToStopAtSendsCompleteCallstackAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  absolute_address_to_size_of_functions_to_stop_at_[kTargetAddress1] = 100;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest,
       VisitSingleFrameStackSampleOutsideAnyFunctionToStopSendsUnwindingErrorAndAddressInfos) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  absolute_address_to_size_of_functions_to_stop_at_[kTargetAddress2] = 100;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest, VisitStackSampleWithinUprobeSendsInUprobesCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kUprobesFrame2, kFrame2};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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
    UprobesUnwindingVisitorTest,
    VisitStackSampleWithinUserSpaceInstrumentationTrampolineSendsInUserSpaceInstrumentationCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kEntryTrampolineFrame, kFrame2};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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
    UprobesUnwindingVisitorTest,
    VisitStackSampleWithinUserSpaceInstrumentationTrampolineAndLibrarySendsInUserSpaceInstrumentationCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1, kUserSpaceLibraryFrame, kFrame3,
                                                kEntryTrampolineFrame};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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
    UprobesUnwindingVisitorTest,
    VisitStackSampleWithinUserSpaceInstrumentationLibraryButNotTrampolineSendsCompleteCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1, kUserSpaceLibraryFrame, kFrame3};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest, VisitStackSampleStoppedAtUprobesSendsPatchingFailedCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1, kUprobesFrame1};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest,
       VisitStackSampleStoppedAtUserSpaceInstrumentationTrampolineSendsPatchingFailedCallstack) {
  StackSamplePerfEvent event = BuildFakeStackSamplePerfEvent();

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack{kFrame1, kReturnTrampolineFrame};

  EXPECT_CALL(unwinder_, Unwind(event.data.pid, nullptr, _, _, event.data.dyn_size, _, _))
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

TEST_F(UprobesUnwindingVisitorTest, VisitValidCallchainSampleWithoutUprobesSendsCallstack) {
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

TEST_F(UprobesUnwindingVisitorTest, VisitSingleFrameCallchainSampleDoesNothing) {
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

TEST_F(UprobesUnwindingVisitorTest, VisitCallchainSampleInsideUprobeCodeSendsInUprobesCallstack) {
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
    UprobesUnwindingVisitorTest,
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
    UprobesUnwindingVisitorTest,
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
    UprobesUnwindingVisitorTest,
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

TEST_F(UprobesUnwindingVisitorTest, VisitPatchableCallchainSampleSendsCompleteCallstack) {
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

TEST_F(UprobesUnwindingVisitorTest, VisitUnpatchableCallchainSampleSendsPatchingFailedCallstack) {
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

TEST_F(UprobesUnwindingVisitorTest,
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
    UprobesUnwindingVisitorTest,
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

class UprobesUnwindingVisitorMmapTest : public ::testing::Test {
 protected:
  void SetUp() override {
    ON_CALL(maps_, Find).WillByDefault([this](uint64_t pc) { return real_maps_->Find(pc); });
    ON_CALL(maps_, Get).WillByDefault([this]() { return real_maps_->Get(); });
    ON_CALL(maps_, AddAndSort)
        .WillByDefault([this](uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
                              const std::string& name) {
          return real_maps_->AddAndSort(start, end, offset, flags, name);
        });
  }

  static constexpr pid_t kPid = 42;

  MockTracerListener listener_;
  MockLibunwindstackMaps maps_;
  UprobesUnwindingVisitor visitor_{&listener_,
                                   &function_call_manager_,
                                   &return_address_manager_,
                                   &maps_,
                                   &unwinder_,
                                   &leaf_function_call_manager_,
                                   /*user_space_instrumentation_addresses=*/nullptr};

 private:
  UprobesFunctionCallManager function_call_manager_;
  MockUprobesReturnAddressManager return_address_manager_{
      /*user_space_instrumentation_addresses=*/nullptr};
  std::unique_ptr<LibunwindstackMaps> real_maps_ = LibunwindstackMaps::ParseMaps("");
  MockLibunwindstackUnwinder unwinder_;
  MockLeafFunctionCallManager leaf_function_call_manager_{128};
};

TEST_F(UprobesUnwindingVisitorMmapTest,
       VisitMmapPerfEventUpdatesLibunwindstackMapsAndSendsModuleUpdates) {
  // 7f4b0c7ab000-7f4b0c845000 r-xp 00000000 00:00 0
  // Anonymous executable mapping.
  MmapPerfEvent anon_mmap_event{
      .timestamp = 1,
      .data =
          {
              .address = 0x7f4b0c7ab000,
              .length = 0x9A000,
              .page_offset = 0,
              .filename = "",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x7f4b0c7ab000, 0x7f4b0c845000, 0, PROT_READ | PROT_EXEC, ""))
      .Times(1);
  EXPECT_CALL(maps_, Find(0x7f4b0c7ab000)).Times(1);
  PerfEvent(std::move(anon_mmap_event)).Accept(&visitor_);

  // 7fffffffe000-7ffffffff000 --xp 00000000 00:00 0    [uprobes]
  // Special anonymous executable mapping.
  MmapPerfEvent special_mmap_event{
      .timestamp = 2,
      .data =
          {
              .address = 0x7fffffffe000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = "[uprobes]",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_,
              AddAndSort(0x7fffffffe000, 0x7ffffffff000, 0, PROT_READ | PROT_EXEC, "[uprobes]"))
      .Times(1);
  PerfEvent(std::move(special_mmap_event)).Accept(&visitor_);

  const std::string test_binary_path = (orbit_test::GetTestdataDir() / "target_fp").string();

  // 55bf53c22000-55bf53c24000 r-xp 00001000 fe:00 60425802    /path/to/target_fp
  // File-backed executable mapping.
  MmapPerfEvent file_mmap_event{
      .timestamp = 3,
      .data =
          {
              .address = 0x55bf53c22000,
              .length = 0x2000,
              .page_offset = 0x1000,
              .filename = test_binary_path,
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x55bf53c22000, 0x55bf53c24000, 0x1000, PROT_READ | PROT_EXEC,
                                test_binary_path))
      .Times(1);
  orbit_grpc_protos::ModuleUpdateEvent actual_module_update;
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(file_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 3);
  EXPECT_EQ(actual_module_update.module().name(), "target_fp");
  EXPECT_EQ(actual_module_update.module().file_path(), test_binary_path);
  EXPECT_EQ(actual_module_update.module().file_size(), 27824);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x55bf53c22000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x55bf53c24000);
  EXPECT_EQ(actual_module_update.module().build_id(), "d7e2447f79faa88528dd0d130ac7cc5f168ca090");
  EXPECT_EQ(actual_module_update.module().load_bias(), 0);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), 0x1000);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kElfFile);

  // 55bf53c24000-55bf53c25000 r--p 00003000 fe:00 60425802    /path/to/target_fp
  // File-backed non-executable mapping.
  MmapPerfEvent file_mmap_data_event{
      .timestamp = 4,
      .data =
          {
              .address = 0x55bf53c24000,
              .length = 0x1000,
              .page_offset = 0x3000,
              .filename = test_binary_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_,
              AddAndSort(0x55bf53c24000, 0x55bf53c25000, 0x3000, PROT_READ, test_binary_path))
      .Times(1);
  PerfEvent(std::move(file_mmap_data_event)).Accept(&visitor_);

  // 7f4b0cabe000-7f4b0cad5000 r-xp 00003000 fe:00 50336180    /path/to/nothing
  // File-backed executable mapping, but the file doesn't exist.
  MmapPerfEvent bad_file_mmap_event{
      .timestamp = 5,
      .data =
          {
              .address = 0x7f4b0cabe000,
              .length = 0x17000,
              .page_offset = 0x3000,
              .filename = "/path/to/nothing",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x7f4b0cabe000, 0x7f4b0cad5000, 0x3000, PROT_READ | PROT_EXEC,
                                "/path/to/nothing"))
      .Times(1);
  PerfEvent(std::move(bad_file_mmap_event)).Accept(&visitor_);
}

TEST_F(UprobesUnwindingVisitorMmapTest,
       VisitMmapPerfEventSendsModuleUpdatesFromPeCoffTextSectionInAnonExecMap) {
  const std::string libtest_path = (orbit_test::GetTestdataDir() / "libtest.dll").string();

  // 100000-101000 r--p 00000000 01:02 42    /path/to/libtest.dll
  MmapPerfEvent headers_mmap_data_event{
      .timestamp = 1,
      .data =
          {
              .address = 0x100000,
              .length = 0x1000,
              .page_offset = 0,
              .filename = libtest_path,
              .executable = false,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x100000, 0x101000, 0, PROT_READ, libtest_path)).Times(1);
  PerfEvent(std::move(headers_mmap_data_event)).Accept(&visitor_);

  // 101000-103000 r-xp 00000000 00:00 0
  MmapPerfEvent text_mmap_event{
      .timestamp = 2,
      .data =
          {
              .address = 0x101000,
              .length = 0x2000,
              .page_offset = 0,
              .filename = "",
              .executable = true,
              .pid = kPid,
          },
  };
  EXPECT_CALL(maps_, AddAndSort(0x101000, 0x103000, 0, PROT_READ | PROT_EXEC, "")).Times(1);
  EXPECT_CALL(maps_, Find(0x101000)).Times(1);
  orbit_grpc_protos::ModuleUpdateEvent actual_module_update;
  EXPECT_CALL(listener_, OnModuleUpdate).Times(1).WillOnce(SaveArg<0>(&actual_module_update));
  PerfEvent(std::move(text_mmap_event)).Accept(&visitor_);
  EXPECT_EQ(actual_module_update.pid(), kPid);
  EXPECT_EQ(actual_module_update.timestamp_ns(), 2);
  EXPECT_EQ(actual_module_update.module().name(), "libtest.dll");
  EXPECT_EQ(actual_module_update.module().file_path(), libtest_path);
  EXPECT_EQ(actual_module_update.module().file_size(), 96441);
  EXPECT_EQ(actual_module_update.module().address_start(), 0x101000);
  EXPECT_EQ(actual_module_update.module().address_end(), 0x103000);
  EXPECT_EQ(actual_module_update.module().build_id(), "");
  EXPECT_EQ(actual_module_update.module().load_bias(), 0x62640000);
  EXPECT_EQ(actual_module_update.module().executable_segment_offset(), 0x1000);
  EXPECT_EQ(actual_module_update.module().soname(), "");
  EXPECT_EQ(actual_module_update.module().object_file_type(),
            orbit_grpc_protos::ModuleInfo::kCoffFile);
}

}  // namespace orbit_linux_tracing
