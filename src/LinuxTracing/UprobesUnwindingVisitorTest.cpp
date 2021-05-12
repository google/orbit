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
using ::testing::Property;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::UnorderedElementsAre;

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
  MOCK_METHOD(std::vector<unwindstack::FrameData>, Unwind,
              (pid_t, unwindstack::Maps*, (const std::array<uint64_t, PERF_REG_X86_64_MAX>&),
               const void*, uint64_t),
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
};

class UprobesUnwindingVisitorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    visitor_ = std::make_unique<UprobesUnwindingVisitor>(&maps_, &unwinder_);
    visitor_->SetListener(&listener_);

    EXPECT_CALL(maps_, Find(AllOf(Ge(kUprobesMapsStart), Lt(kUprobesMapsEnd))))
        .WillRepeatedly(Return(&kUprobesMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kTargetMapsStart), Lt(kTargetMapsEnd))))
        .WillRepeatedly(Return(&kTargetMapInfo));

    EXPECT_CALL(maps_, Find(AllOf(Ge(kNonExecutableMapsStart), Lt(kNonExecutableMapsEnd))))
        .WillRepeatedly(Return(&non_executable_map_info_));
  }

  void TearDown() override { visitor_.reset(); }

  MockLibunwindstackMaps maps_;
  MockLibunwindstackUnwinder unwinder_;
  MockTracerListener listener_;

  std::unique_ptr<UprobesUnwindingVisitor> visitor_ = nullptr;

  static constexpr uint64_t kUprobesMapsStart = 42;
  static constexpr uint64_t kUprobesMapsEnd = 84;

  static constexpr uint64_t kTargetMapsStart = 100;
  static constexpr uint64_t kTargetMapsEnd = 200;

  static constexpr uint64_t kNonExecutableMapsStart = 500;
  static constexpr uint64_t kNonExecutableMapsEnd = 600;

  static constexpr uint64_t kTargetAddress1 = 100;
  //  static constexpr uint64_t kUprobesAddress = 42;
  static constexpr uint64_t kTargetAddress2 = 200;
  static constexpr uint64_t kTargetAddress3 = 300;
  //  static constexpr uint64_t kNonExecutableAddress3 = 500;

  static inline const std::string kUprobesName = "[uprobes]";
  static inline const std::string kTargetName = "target";
  static inline const std::string kNonExecutableName = "data";

  static inline unwindstack::MapInfo kUprobesMapInfo{
      nullptr, kUprobesMapsStart, kUprobesMapsEnd, 0, PROT_EXEC | PROT_READ, kUprobesName};

  static inline unwindstack::MapInfo kTargetMapInfo{nullptr, kTargetMapsStart,      kTargetMapsEnd,
                                                    0,       PROT_EXEC | PROT_READ, kTargetName};

  static inline unwindstack::MapInfo non_executable_map_info_{
      nullptr, kNonExecutableMapsStart, kNonExecutableMapsEnd,
      0,       PROT_EXEC | PROT_READ,   kNonExecutableName};
};

}  // namespace

TEST_F(UprobesUnwindingVisitorTest, VisitValidStackSampleWithoutUprobesEmitsEvents) {
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
  event.ring_buffer_record->sample_id = sample_id;

  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> libunwindstack_callstack;

  unwindstack::FrameData frame_1{
      .pc = kTargetAddress1,
      .function_name = "foo",
      .function_offset = 0,
      .map_name = kTargetName,
  };
  libunwindstack_callstack.push_back(frame_1);
  unwindstack::FrameData frame_2{
      .pc = kTargetAddress2,
      .function_name = "bar",
      .function_offset = 0,
      .map_name = kTargetName,
  };
  libunwindstack_callstack.push_back(frame_2);
  unwindstack::FrameData frame_3{
      .pc = kTargetAddress3,
      .function_name = "baz",
      .function_offset = 0,
      .map_name = kTargetName,
  };
  libunwindstack_callstack.push_back(frame_3);
  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackSize))
      .Times(1)
      .WillOnce(Return(libunwindstack_callstack));

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

  visitor_->visit(&event);

  EXPECT_THAT(actual_callstack_sample.callstack().pcs(),
              ElementsAre(kTargetAddress1, kTargetAddress2, kTargetAddress3));
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

TEST_F(UprobesUnwindingVisitorTest, VisitInvalidStackSampleWithoutUprobesLeadsToUnwindingError) {
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
  event.ring_buffer_record->sample_id = sample_id;

  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> empty_callstack;

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackSize))
      .Times(1)
      .WillOnce(Return(empty_callstack));

  EXPECT_CALL(listener_, OnCallstackSample).Times(0);
  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->visit(&event);

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

TEST_F(UprobesUnwindingVisitorTest,
       VisitSingleFrameStackSampleWithoutUprobesLeadsToUnwindingError) {
  constexpr uint32_t kPid = 10;
  constexpr uint64_t kStackSize = 13;
  StackSamplePerfEvent event{kStackSize};
  perf_event_sample_id_tid_time_streamid_cpu sample_id{
      .pid = kPid, .tid = 11, .time = 15, .stream_id = 12, .cpu = 0, .res = 0};
  event.ring_buffer_record->sample_id = sample_id;

  EXPECT_CALL(maps_, Get).Times(1).WillOnce(Return(nullptr));

  std::vector<unwindstack::FrameData> incomplete_callstack;
  unwindstack::FrameData frame_1{
      .pc = kTargetAddress1,
      .function_name = "foo",
      .function_offset = 0,
      .map_name = kTargetName,
  };
  incomplete_callstack.push_back(frame_1);

  EXPECT_CALL(unwinder_, Unwind(kPid, nullptr, _, _, kStackSize))
      .Times(1)
      .WillOnce(Return(incomplete_callstack));

  EXPECT_CALL(listener_, OnCallstackSample).Times(0);
  EXPECT_CALL(listener_, OnAddressInfo).Times(0);

  std::atomic<uint64_t> unwinding_errors = 0;
  std::atomic<uint64_t> discarded_samples_in_uretprobes_counter = 0;
  visitor_->SetUnwindErrorsAndDiscardedSamplesCounters(&unwinding_errors,
                                                       &discarded_samples_in_uretprobes_counter);

  visitor_->visit(&event);

  EXPECT_EQ(unwinding_errors, 1);
  EXPECT_EQ(discarded_samples_in_uretprobes_counter, 0);
}

}  // namespace orbit_linux_tracing