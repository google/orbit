// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/mman.h>

#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
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

class MockTracerListener : public TracerListener {
 public:
  MOCK_METHOD(void, OnSchedulingSlice, (orbit_grpc_protos::SchedulingSlice), (override));
  MOCK_METHOD(void, OnCallstackSample, (orbit_grpc_protos::FullCallstackSample), (override));
  MOCK_METHOD(void, OnFunctionCall, (orbit_grpc_protos::FunctionCall), (override));
  MOCK_METHOD(void, OnIntrospectionScope, (orbit_grpc_protos::IntrospectionScope), (override));
  MOCK_METHOD(void, OnGpuJob, (orbit_grpc_protos::FullGpuJob), (override));
  MOCK_METHOD(void, OnThreadName, (orbit_grpc_protos::ThreadName), (override));
  MOCK_METHOD(void, OnThreadNamesSnapshot, (orbit_grpc_protos::ThreadNamesSnapshot), (override));
  MOCK_METHOD(void, OnThreadStateSlice, (orbit_grpc_protos::ThreadStateSlice), (override));
  MOCK_METHOD(void, OnAddressInfo, (orbit_grpc_protos::FullAddressInfo), (override));
  MOCK_METHOD(void, OnTracepointEvent, (orbit_grpc_protos::FullTracepointEvent), (override));
  MOCK_METHOD(void, OnModulesSnapshot, (orbit_grpc_protos::ModulesSnapshot), (override));
  MOCK_METHOD(void, OnModuleUpdate, (orbit_grpc_protos::ModuleUpdateEvent), (override));
  MOCK_METHOD(void, OnErrorsWithPerfEventOpenEvent,
              (orbit_grpc_protos::ErrorsWithPerfEventOpenEvent), (override));
  MOCK_METHOD(void, OnLostPerfRecordsEvent, (orbit_grpc_protos::LostPerfRecordsEvent), (override));
  MOCK_METHOD(void, OnOutOfOrderEventsDiscardedEvent,
              (orbit_grpc_protos::OutOfOrderEventsDiscardedEvent), (override));
};

class MockUprobesReturnAddressManager : public UprobesReturnAddressManager {
 public:
  MOCK_METHOD(void, ProcessUprobes, (pid_t, uint64_t, uint64_t), (override));
  MOCK_METHOD(void, PatchSample, (pid_t, uint64_t, void*, uint64_t), (override));
  MOCK_METHOD(bool, PatchCallchain, (pid_t, uint64_t*, uint64_t, LibunwindstackMaps*), (override));
  MOCK_METHOD(void, ProcessUretprobes, (pid_t), (override));
};

class MockLeafFunctionCallManager : public LeafFunctionCallManager {
 public:
  explicit MockLeafFunctionCallManager(uint16_t stack_dump_size)
      : LeafFunctionCallManager(stack_dump_size) {}
  MOCK_METHOD(Callstack::CallstackType, PatchCallerOfLeafFunction,
              (CallchainSamplePerfEvent*, LibunwindstackMaps*, LibunwindstackUnwinder*),
              (override));
};

class UprobesUnwindingVisitorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    visitor_ = std::make_unique<UprobesUnwindingVisitor>(&listener_, &function_call_manager_,
                                                         &return_address_manager_, &maps_,
                                                         &unwinder_, &leaf_function_call_manager_);

    EXPECT_CALL(maps_, Find(AllOf(Ge(kUprobesMapsStart), Lt(kUprobesMapsEnd))))
        .WillRepeatedly(Return(&kUprobesMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kTargetMapsStart), Lt(kTargetMapsEnd))))
        .WillRepeatedly(Return(&kTargetMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kNonExecutableMapsStart), Lt(kNonExecutableMapsEnd))))
        .WillRepeatedly(Return(&non_executable_map_info_));
  }

  void TearDown() override { visitor_.reset(); }

  static constexpr uint32_t kStackDumpSize = 128;
  UprobesFunctionCallManager function_call_manager_;
  MockUprobesReturnAddressManager return_address_manager_;
  MockLibunwindstackMaps maps_;
  MockLibunwindstackUnwinder unwinder_;
  MockLeafFunctionCallManager leaf_function_call_manager_{kStackDumpSize};
  MockTracerListener listener_;

  std::unique_ptr<UprobesUnwindingVisitor> visitor_ = nullptr;

  static constexpr uint64_t kUprobesMapsStart = 42;
  static constexpr uint64_t kUprobesMapsEnd = 84;

  static constexpr uint64_t kTargetMapsStart = 100;
  static constexpr uint64_t kTargetMapsEnd = 200;

  static constexpr uint64_t kNonExecutableMapsStart = 500;
  static constexpr uint64_t kNonExecutableMapsEnd = 600;

  static constexpr uint64_t kKernelAddress = 11;

  static constexpr uint64_t kTargetAddress1 = 100;
  //  static constexpr uint64_t kUprobesAddress = 42;
  static constexpr uint64_t kTargetAddress2 = 200;
  static constexpr uint64_t kTargetAddress3 = 300;
  //  static constexpr uint64_t kNonExecutableAddress3 = 500;

  static inline const std::string kUprobesName = "[uprobes]";
  static inline const std::string kTargetName = "target";
  static inline const std::string kNonExecutableName = "data";

  static inline unwindstack::MapInfo kUprobesMapInfo{
      nullptr, nullptr, kUprobesMapsStart, kUprobesMapsEnd, 0, PROT_EXEC | PROT_READ, kUprobesName};

  static inline unwindstack::MapInfo kTargetMapInfo{
      nullptr, nullptr, kTargetMapsStart, kTargetMapsEnd, 0, PROT_EXEC | PROT_READ, kTargetName};

  static inline unwindstack::MapInfo non_executable_map_info_{nullptr,
                                                              nullptr,
                                                              kNonExecutableMapsStart,
                                                              kNonExecutableMapsEnd,
                                                              0,
                                                              PROT_EXEC | PROT_READ,
                                                              kNonExecutableName};

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
};

}  // namespace

TEST_F(UprobesUnwindingVisitorTest,
       VisitUprobesAndUretprobesPerfEventsInVariousCombinationsSendsFunctionCalls) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  constexpr uint32_t kCpu = 1;

  {
    Function function1{1, "/path/to/module", 0x01, false, false};
    UprobesPerfEvent uprobe1;
    uprobe1.ring_buffer_record.sample_id.pid = kPid;
    uprobe1.ring_buffer_record.sample_id.tid = kTid;
    uprobe1.ring_buffer_record.sample_id.time = 100;
    uprobe1.ring_buffer_record.sample_id.cpu = kCpu;
    uprobe1.ring_buffer_record.regs.sp = 0x40;
    uprobe1.ring_buffer_record.regs.ip = 0x01;
    uprobe1.ring_buffer_record.stack.top8bytes = 0x00;
    uprobe1.SetFunction(&function1);

    EXPECT_CALL(return_address_manager_, ProcessUprobes(kTid, 0x40, 0x00)).Times(1);
    visitor_->Visit(&uprobe1);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    Function function2{2, "/path/to/module", 0x02, true, false};
    UprobesWithArgumentsPerfEvent uprobe2;
    uprobe2.ring_buffer_record.sample_id.pid = kPid;
    uprobe2.ring_buffer_record.sample_id.tid = kTid;
    uprobe2.ring_buffer_record.sample_id.time = 200;
    uprobe2.ring_buffer_record.sample_id.cpu = kCpu;
    uprobe2.ring_buffer_record.regs.sp = 0x30;
    uprobe2.ring_buffer_record.regs.ip = 0x02;
    uprobe2.ring_buffer_record.stack.top8bytes = 0x01;
    uprobe2.ring_buffer_record.regs.di = 1;
    uprobe2.ring_buffer_record.regs.si = 2;
    uprobe2.ring_buffer_record.regs.dx = 3;
    uprobe2.ring_buffer_record.regs.cx = 4;
    uprobe2.ring_buffer_record.regs.r8 = 5;
    uprobe2.ring_buffer_record.regs.r9 = 6;
    uprobe2.SetFunction(&function2);

    EXPECT_CALL(return_address_manager_, ProcessUprobes(kTid, 0x30, 0x01)).Times(1);
    visitor_->Visit(&uprobe2);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    Function function3{3, "/path/to/module", 0x03, false, true};
    UprobesPerfEvent uprobe3;
    uprobe3.ring_buffer_record.sample_id.pid = kPid;
    uprobe3.ring_buffer_record.sample_id.tid = kTid;
    uprobe3.ring_buffer_record.sample_id.time = 300;
    uprobe3.ring_buffer_record.sample_id.cpu = kCpu;
    uprobe3.ring_buffer_record.regs.sp = 0x20;
    uprobe3.ring_buffer_record.regs.ip = 0x03;
    uprobe3.ring_buffer_record.stack.top8bytes = 0x02;
    uprobe3.SetFunction(&function3);

    EXPECT_CALL(return_address_manager_, ProcessUprobes(kTid, 0x20, 0x02)).Times(1);
    visitor_->Visit(&uprobe3);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    Function function4{4, "/path/to/module", 0x04, true, true};
    UprobesWithArgumentsPerfEvent uprobe4;
    uprobe4.ring_buffer_record.sample_id.pid = kPid;
    uprobe4.ring_buffer_record.sample_id.tid = kTid;
    uprobe4.ring_buffer_record.sample_id.time = 400;
    uprobe4.ring_buffer_record.sample_id.cpu = kCpu;
    uprobe4.ring_buffer_record.regs.sp = 0x10;
    uprobe4.ring_buffer_record.regs.ip = 0x04;
    uprobe4.ring_buffer_record.stack.top8bytes = 0x03;
    uprobe4.ring_buffer_record.regs.di = 1;
    uprobe4.ring_buffer_record.regs.si = 2;
    uprobe4.ring_buffer_record.regs.dx = 3;
    uprobe4.ring_buffer_record.regs.cx = 4;
    uprobe4.ring_buffer_record.regs.r8 = 5;
    uprobe4.ring_buffer_record.regs.r9 = 6;
    uprobe4.SetFunction(&function4);

    EXPECT_CALL(return_address_manager_, ProcessUprobes(kTid, 0x10, 0x03)).Times(1);
    visitor_->Visit(&uprobe4);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UretprobesWithReturnValuePerfEvent uretprobe4;
    uretprobe4.ring_buffer_record.sample_id.pid = kPid;
    uretprobe4.ring_buffer_record.sample_id.tid = kTid;
    uretprobe4.ring_buffer_record.sample_id.time = 500;
    uretprobe4.ring_buffer_record.regs.ax = 456;

    EXPECT_CALL(return_address_manager_, ProcessUretprobes(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    visitor_->Visit(&uretprobe4);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 4);
    EXPECT_EQ(actual_function_call.duration_ns(), 100);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 500);
    EXPECT_EQ(actual_function_call.depth(), 3);
    EXPECT_EQ(actual_function_call.return_value(), 456);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre(1, 2, 3, 4, 5, 6));
  }

  {
    UretprobesWithReturnValuePerfEvent uretprobe3;
    uretprobe3.ring_buffer_record.sample_id.pid = kPid;
    uretprobe3.ring_buffer_record.sample_id.tid = kTid;
    uretprobe3.ring_buffer_record.sample_id.time = 600;
    uretprobe3.ring_buffer_record.regs.ax = 123;

    EXPECT_CALL(return_address_manager_, ProcessUretprobes(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    visitor_->Visit(&uretprobe3);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 3);
    EXPECT_EQ(actual_function_call.duration_ns(), 300);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 600);
    EXPECT_EQ(actual_function_call.depth(), 2);
    EXPECT_EQ(actual_function_call.return_value(), 123);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }

  {
    UretprobesPerfEvent uretprobe2;
    uretprobe2.ring_buffer_record.sample_id.pid = kPid;
    uretprobe2.ring_buffer_record.sample_id.tid = kTid;
    uretprobe2.ring_buffer_record.sample_id.time = 700;

    EXPECT_CALL(return_address_manager_, ProcessUretprobes(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    visitor_->Visit(&uretprobe2);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 2);
    EXPECT_EQ(actual_function_call.duration_ns(), 500);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 700);
    EXPECT_EQ(actual_function_call.depth(), 1);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre(1, 2, 3, 4, 5, 6));
  }

  {
    UretprobesPerfEvent uretprobe1;
    uretprobe1.ring_buffer_record.sample_id.pid = kPid;
    uretprobe1.ring_buffer_record.sample_id.tid = kTid;
    uretprobe1.ring_buffer_record.sample_id.time = 800;

    EXPECT_CALL(return_address_manager_, ProcessUretprobes(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    visitor_->Visit(&uretprobe1);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 1);
    EXPECT_EQ(actual_function_call.duration_ns(), 700);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 800);
    EXPECT_EQ(actual_function_call.depth(), 0);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }
}

//-------------------------------//
// VISIT STACK SAMPLE PERF EVENT //
//-------------------------------//

TEST_F(UprobesUnwindingVisitorTest,
       VisitValidStackSampleWithoutUprobesSendsCompleteCallstackAndAddressInfos) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;
  StackSamplePerfEvent event{kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack;

  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kFrame2);
  libunwindstack_callstack.push_back(kFrame3);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackSize, _, _))
      .Times(1)
      .WillOnce(Return(
          LibunwindstackResult{libunwindstack_callstack, unwindstack::ErrorCode::ERROR_NONE}));

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
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, "foo"),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress2),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, "bar"),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName)),
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress3),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, "baz"),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest, VisitEmptyStackSampleWithoutUprobesDoesNothing) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;
  StackSamplePerfEvent event{kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> empty_callstack;

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackSize, _, _))
      .Times(1)
      .WillOnce(Return(
          LibunwindstackResult{empty_callstack, unwindstack::ErrorCode::ERROR_MEMORY_INVALID}));

  EXPECT_CALL(listener_, OnCallstackSample).Times(0);

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest,
       VisitInvalidStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfo) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;
  StackSamplePerfEvent event{kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillRepeatedly(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack;
  libunwindstack_callstack.push_back(kFrame1);
  libunwindstack_callstack.push_back(kFrame2);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackSize, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{libunwindstack_callstack,
                                            unwindstack::ErrorCode::ERROR_MEMORY_INVALID}));

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
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  // On unwinding errors, only the first frame is added to the Callstack.
  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kDwarfUnwindingError);
  EXPECT_THAT(actual_address_infos,
              UnorderedElementsAre(AllOf(
                  Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                  Property(&orbit_grpc_protos::FullAddressInfo::function_name, "foo"),
                  Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                  Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest,
       VisitSingleFrameStackSampleWithoutUprobesSendsUnwindingErrorAndAddressInfo) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;
  StackSamplePerfEvent event{kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> incomplete_callstack;
  incomplete_callstack.push_back(kFrame1);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackSize, _, _))
      .Times(1)
      .WillOnce(
          Return(LibunwindstackResult{incomplete_callstack, unwindstack::ErrorCode::ERROR_NONE}));

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
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kDwarfUnwindingError);
  EXPECT_THAT(actual_address_infos,
              UnorderedElementsAre(AllOf(
                  Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kTargetAddress1),
                  Property(&orbit_grpc_protos::FullAddressInfo::function_name, "foo"),
                  Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                  Property(&orbit_grpc_protos::FullAddressInfo::module_name, kTargetName))));

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest, VisitStackSampleWithinUprobeSendsInUprobesCallstack) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;
  StackSamplePerfEvent event{kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid,
      .tid = 11,
      .time = 15,
      .stream_id = 12,
      .cpu = 0,
      .res = 0,
  };
  event.ring_buffer_record.sample_id = sample_id;

  EXPECT_CALL(return_address_manager_, PatchSample).Times(1).WillOnce(Return());
  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> callstack;
  unwindstack::FrameData frame_1{
      .pc = kUprobesMapsStart,
      .function_name = "uprobe",
      .function_offset = 0,
      .map_name = kUprobesName,
      .map_start = kUprobesMapsStart,
  };
  callstack.push_back(frame_1);
  callstack.push_back(kFrame2);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackSize, _, _))
      .Times(1)
      .WillOnce(Return(LibunwindstackResult{callstack, unwindstack::ErrorCode::ERROR_NONE}));

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
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kUprobesMapsStart));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kInUprobes);
  EXPECT_THAT(
      actual_address_infos,
      UnorderedElementsAre(
          AllOf(Property(&orbit_grpc_protos::FullAddressInfo::absolute_address, kUprobesMapsStart),
                Property(&orbit_grpc_protos::FullAddressInfo::function_name, "[uprobes]"),
                Property(&orbit_grpc_protos::FullAddressInfo::offset_in_function, 0),
                Property(&orbit_grpc_protos::FullAddressInfo::module_name, "[uprobes]"))));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 1);
}

//-----------------------------------//
// VISIT CALLCHAIN SAMPLE PERF EVENT //
//-----------------------------------//

TEST_F(UprobesUnwindingVisitorTest, VisitValidCallchainSampleWithoutUprobesSendsCallstack) {
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

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(&kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillOnce(Return(true));
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kComplete));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest, VisitSingleFrameCallchainSampleDoesNothing) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);

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

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(&kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  EXPECT_CALL(listener_, OnCallstackSample).Times(0);

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest, VisitCallchainSampleInsideUprobeCodeSendsInUprobesCallstack) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kUprobesMapsStart);
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

  EXPECT_CALL(maps_, Find(_)).WillRepeatedly(Return(&kTargetMapInfo));
  EXPECT_CALL(maps_, Find(kUprobesMapsStart)).WillRepeatedly(Return(&kUprobesMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction).Times(0);

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kUprobesMapsStart));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kInUprobes);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 1);
}

TEST_F(UprobesUnwindingVisitorTest, VisitCallchainSampleWithUprobeSendsCompleteCallstack) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kUprobesMapsStart + 1);
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

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(&kTargetMapInfo));
  auto fake_patch_callchain = [](pid_t /*tid*/, uint64_t* callchain, uint64_t callchain_size,
                                 orbit_linux_tracing::LibunwindstackMaps *
                                 /*maps*/) -> bool {
    CHECK(callchain != nullptr);
    CHECK(callchain_size == 4);
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
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
  EXPECT_EQ(actual_callstack_sample.callstack().type(), orbit_grpc_protos::Callstack::kComplete);

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest,
       VisitCallchainSampleWithBrokenUprobeSendsPatchingFailedCallstack) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;

  std::vector<uint64_t> callchain;
  callchain.push_back(kKernelAddress);
  callchain.push_back(kTargetAddress1);
  // Increment by one as the return address is the next address.
  callchain.push_back(kUprobesMapsStart + 1);
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

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(&kTargetMapInfo));
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kComplete));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillOnce(Return(false));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kUprobesPatchingFailed);

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest,
       VisitLeafCallOptimizedCallchainSampleWithoutUprobesSendsCompleteCallstack) {
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

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(&kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(1).WillRepeatedly(Return(true));

  auto fake_patch_caller_of_leaf_function = [](CallchainSamplePerfEvent* event,
                                               LibunwindstackMaps* /*maps*/,
                                               orbit_linux_tracing::LibunwindstackUnwinder *
                                               /*unwinder*/) -> Callstack::CallstackType {
    CHECK(event != nullptr);
    std::vector<uint64_t> patched_callchain;
    EXPECT_THAT(event->ips, ElementsAre(kKernelAddress, kTargetAddress1, kTargetAddress3 + 1));
    patched_callchain.push_back(kKernelAddress);
    patched_callchain.push_back(kTargetAddress1);
    // Patch in the missing frame:
    patched_callchain.push_back(kTargetAddress2 + 1);
    patched_callchain.push_back(kTargetAddress3 + 1);
    event->ring_buffer_record.nr = patched_callchain.size();
    event->ips = std::move(patched_callchain);
    return Callstack::kComplete;
  };
  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Invoke(fake_patch_caller_of_leaf_function));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));

  EXPECT_EQ(unwinding_errors, 0);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(
    UprobesUnwindingVisitorTest,
    VisitLeafCallOptimizedCallchainSampleWherePatchingLeafFunctionCallerFailsSendsFramePointerUnwindingErrorCallstack) {
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

  EXPECT_CALL(maps_, Find).WillRepeatedly(Return(&kTargetMapInfo));
  EXPECT_CALL(return_address_manager_, PatchCallchain).Times(0);

  EXPECT_CALL(leaf_function_call_manager_, PatchCallerOfLeafFunction)
      .Times(1)
      .WillOnce(Return(Callstack::kFramePointerUnwindingError));

  orbit_grpc_protos::FullCallstackSample actual_callstack_sample;
  EXPECT_CALL(listener_, OnCallstackSample).Times(1).WillOnce(SaveArg<0>(&actual_callstack_sample));

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->Visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(), ElementsAre(kTargetAddress1));
  EXPECT_EQ(actual_callstack_sample.callstack().type(),
            orbit_grpc_protos::Callstack::kFramePointerUnwindingError);

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

}  // namespace orbit_linux_tracing