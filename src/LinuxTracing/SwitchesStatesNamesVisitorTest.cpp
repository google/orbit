// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "PerfEvent.h"
#include "SwitchesStatesNamesVisitor.h"

using ::testing::SaveArg;

using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;

namespace orbit_linux_tracing {

TEST(SwitchesStatesNamesVisitor, NeedsTracerListener) {
  EXPECT_DEATH(SwitchesStatesNamesVisitor{nullptr}, "");
}

namespace {

class MockTracerListener : public orbit_tracing_interface::TracerListener {
 public:
  MOCK_METHOD(void, OnSchedulingSlice, (orbit_grpc_protos::SchedulingSlice), (override));
  MOCK_METHOD(void, OnCallstackSample, (orbit_grpc_protos::FullCallstackSample), (override));
  MOCK_METHOD(void, OnFunctionCall, (orbit_grpc_protos::FunctionCall), (override));
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

class SwitchesStatesNamesVisitorTest : public ::testing::Test {
 protected:
  SwitchesStatesNamesVisitorTest() { visitor_.SetThreadStateCounter(&thread_state_counter_); }

  MockTracerListener mock_listener_;
  SwitchesStatesNamesVisitor visitor_{&mock_listener_};
  std::atomic<uint64_t> thread_state_counter_ = 0;

  // For almost all tests.
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;

  // For ThreadName-related tests.
  static constexpr const char* kLongComm = "max_length_comm";
  static constexpr const char* kShortComm = "comm";
  static constexpr uint64_t kTimestampNs = 100;

  // For both SchedulingSlice-related and ThreadStateSlice-related tests.
  static constexpr pid_t kPrevTid = 11;
  static constexpr pid_t kNextTid = 22;

  // For SchedulingSlice-related tests.
  static constexpr pid_t kMinusOnePid = -1;
  static constexpr uint32_t kCpu = 1;
  static constexpr uint64_t kInTimestampNs = 100;
  static constexpr uint64_t kOutTimestampNs = 101;

  // For ThreadStateSlice-related tests.
  static constexpr pid_t kTid1 = kTid;
  static constexpr pid_t kTid2 = 43;
  static constexpr const char* kComm2 = "comm2";
  static constexpr uint32_t kCpu1 = kCpu;
  static constexpr uint32_t kCpu2 = 2;
  static constexpr uint64_t kStartTimestampNs = 100;
  static constexpr uint64_t kWakeTimestampNs1 = 110;
  static constexpr uint64_t kInTimestampNs1 = 111;
  static constexpr uint64_t kOutTimestampNs1 = 112;
  static constexpr uint64_t kNewTimestampNs2 = 120;
  static constexpr uint64_t kInTimestampNs2 = 121;
  static constexpr uint64_t kOutTimestampNs2 = 122;
  static constexpr uint64_t kWakeTimestampNs2 = 123;
  static constexpr uint64_t kStopTimestampNs = 130;
  static constexpr int64_t kRunnableStateMask = 0;
  static constexpr int64_t kInterruptibleSleepStateMask = 0x01;
  static constexpr int64_t kDeadStateMask = 0x10;

  void ProcessFakeEventsForThreadStateTests();
};

std::unique_ptr<ForkPerfEvent> MakeFakeForkPerfEvent(pid_t pid, pid_t tid) {
  auto event = std::make_unique<ForkPerfEvent>();
  event->ring_buffer_record.pid = pid;
  CHECK(event->GetPid() == pid);
  event->ring_buffer_record.tid = tid;
  CHECK(event->GetTid() == tid);
  return event;
}

std::unique_ptr<ExitPerfEvent> MakeFakeExitPerfEvent(pid_t pid, pid_t tid) {
  auto event = std::make_unique<ExitPerfEvent>();
  event->ring_buffer_record.pid = pid;
  CHECK(event->GetPid() == pid);
  event->ring_buffer_record.tid = tid;
  CHECK(event->GetTid() == tid);
  return event;
}

std::unique_ptr<TaskNewtaskPerfEvent> MakeFakeTaskNewtaskPerfEvent(pid_t new_tid, const char* comm,
                                                                   uint64_t timestamp_ns) {
  auto event = std::make_unique<TaskNewtaskPerfEvent>();
  event->ring_buffer_record.data.pid = new_tid;
  CHECK(event->GetNewTid() == new_tid);
  CHECK(strlen(comm) < 16);
  strncpy(event->ring_buffer_record.data.comm, comm, 16);
  CHECK(strcmp(event->GetComm(), comm) == 0);
  event->ring_buffer_record.sample_id.time = timestamp_ns;
  CHECK(event->GetTimestamp() == timestamp_ns);
  return event;
}

std::unique_ptr<TaskRenamePerfEvent> MakeFakeTaskRenamePerfEvent(pid_t renamed_tid,
                                                                 const char* new_comm,
                                                                 uint64_t timestamp_ns) {
  auto event = std::make_unique<TaskRenamePerfEvent>();
  event->ring_buffer_record.data.pid = renamed_tid;
  CHECK(event->GetRenamedTid() == renamed_tid);
  CHECK(strlen(new_comm) < 16);
  strncpy(event->ring_buffer_record.data.newcomm, new_comm, 16);
  CHECK(strcmp(event->GetNewComm(), new_comm) == 0);
  event->ring_buffer_record.sample_id.time = timestamp_ns;
  CHECK(event->GetTimestamp() == timestamp_ns);
  return event;
}

std::unique_ptr<SchedSwitchPerfEvent> MakeFakeSchedSwitchPerfEvent(
    uint32_t cpu, pid_t prev_pid_or_minus_one, pid_t prev_tid, int64_t prev_state_mask,
    pid_t next_tid, uint64_t timestamp_ns) {
  auto event = std::make_unique<SchedSwitchPerfEvent>();
  event->ring_buffer_record.sample_id.cpu = cpu;
  CHECK(event->GetCpu() == cpu);
  event->ring_buffer_record.sample_id.pid = prev_pid_or_minus_one;
  CHECK(event->GetPrevPidOrMinusOne() == prev_pid_or_minus_one);
  event->ring_buffer_record.data.prev_pid = prev_tid;
  CHECK(event->GetPrevTid() == prev_tid);
  event->ring_buffer_record.data.prev_state = prev_state_mask;
  CHECK(event->GetPrevState() == prev_state_mask);
  event->ring_buffer_record.data.next_pid = next_tid;
  CHECK(event->GetNextTid() == next_tid);
  event->ring_buffer_record.sample_id.time = timestamp_ns;
  CHECK(event->GetTimestamp() == timestamp_ns);
  return event;
}

std::unique_ptr<SchedWakeupPerfEvent> MakeFakeSchedWakeupPerfEvent(pid_t woken_tid,
                                                                   uint64_t timestamp_ns) {
  auto event = std::make_unique<SchedWakeupPerfEvent>();
  event->ring_buffer_record.data.pid = woken_tid;
  CHECK(event->GetWokenTid() == woken_tid);
  event->ring_buffer_record.sample_id.time = timestamp_ns;
  CHECK(event->GetTimestamp() == timestamp_ns);
  return event;
}

SchedulingSlice MakeSchedulingSlice(uint32_t pid, uint32_t tid, int32_t core, uint64_t duration_ns,
                                    uint64_t out_timestamp_ns) {
  SchedulingSlice scheduling_slice;
  scheduling_slice.set_pid(pid);
  scheduling_slice.set_tid(tid);
  scheduling_slice.set_core(core);
  scheduling_slice.set_duration_ns(duration_ns);
  scheduling_slice.set_out_timestamp_ns(out_timestamp_ns);
  return scheduling_slice;
}

::testing::Matcher<SchedulingSlice> SchedulingSliceEq(const SchedulingSlice& expected) {
  return ::testing::AllOf(
      ::testing::Property("pid", &SchedulingSlice::pid, expected.pid()),
      ::testing::Property("tid", &SchedulingSlice::tid, expected.tid()),
      ::testing::Property("core", &SchedulingSlice::core, expected.core()),
      ::testing::Property("duration_ns", &SchedulingSlice::duration_ns, expected.duration_ns()),
      ::testing::Property("out_timestamp_ns", &SchedulingSlice::out_timestamp_ns,
                          expected.out_timestamp_ns()));
}

ThreadStateSlice MakeThreadStateSlice(uint32_t tid, ThreadStateSlice::ThreadState thread_state,
                                      uint64_t duration_ns, uint64_t end_timestamp_ns) {
  ThreadStateSlice thread_state_slice;
  thread_state_slice.set_tid(tid);
  thread_state_slice.set_thread_state(thread_state);
  thread_state_slice.set_duration_ns(duration_ns);
  thread_state_slice.set_end_timestamp_ns(end_timestamp_ns);
  return thread_state_slice;
}

::testing::Matcher<ThreadStateSlice> ThreadStateSliceEq(const ThreadStateSlice& expected) {
  return ::testing::AllOf(
      ::testing::Property("tid", &ThreadStateSlice::tid, expected.tid()),
      ::testing::Property("thread_state", &ThreadStateSlice::thread_state, expected.thread_state()),
      ::testing::Property("duration_ns", &ThreadStateSlice::duration_ns, expected.duration_ns()),
      ::testing::Property("end_timestamp_ns", &ThreadStateSlice::end_timestamp_ns,
                          expected.end_timestamp_ns()));
}

ThreadName MakeThreadName(uint32_t pid, uint32_t tid, std::string name, uint64_t timestamp_ns) {
  ThreadName thread_name;
  thread_name.set_pid(pid);
  thread_name.set_tid(tid);
  thread_name.set_name(name);
  thread_name.set_timestamp_ns(timestamp_ns);
  return thread_name;
}

::testing::Matcher<ThreadName> ThreadNameEq(const ThreadName& expected) {
  return ::testing::AllOf(
      ::testing::Property("pid", &ThreadName::pid, expected.pid()),
      ::testing::Property("tid", &ThreadName::tid, expected.tid()),
      ::testing::Property("name", &ThreadName::name, expected.name()),
      ::testing::Property("timestamp_ns", &ThreadName::timestamp_ns, expected.timestamp_ns()));
}

}  // namespace

TEST_F(SwitchesStatesNamesVisitorTest, TaskNewtaskOfUnknownPidCausesThreadNameWithMinusOnePid) {
  ThreadName actual_thread_name;
  EXPECT_CALL(mock_listener_, OnThreadName).Times(1).WillOnce(SaveArg<0>(&actual_thread_name));

  MakeFakeTaskNewtaskPerfEvent(kTid, kLongComm, kTimestampNs)->Accept(&visitor_);
  ThreadName expected_thread_name = MakeThreadName(-1, kTid, kLongComm, kTimestampNs);
  EXPECT_THAT(actual_thread_name, ThreadNameEq(expected_thread_name));
}

TEST_F(SwitchesStatesNamesVisitorTest,
       TaskNewtaskOfTidWithInitialTidToPidAssociationCausesThreadNameWithPid) {
  visitor_.ProcessInitialTidToPidAssociation(kTid, kPid);

  ThreadName actual_thread_name;
  EXPECT_CALL(mock_listener_, OnThreadName).Times(1).WillOnce(SaveArg<0>(&actual_thread_name));

  MakeFakeTaskNewtaskPerfEvent(kTid, kLongComm, kTimestampNs)->Accept(&visitor_);
  ThreadName expected_thread_name = MakeThreadName(kPid, kTid, kLongComm, kTimestampNs);
  EXPECT_THAT(actual_thread_name, ThreadNameEq(expected_thread_name));
}

TEST_F(SwitchesStatesNamesVisitorTest, TaskRenameOfUnknownPidCausesThreadNameWithMinusOnePid) {
  ThreadName actual_thread_name;
  EXPECT_CALL(mock_listener_, OnThreadName).Times(1).WillOnce(SaveArg<0>(&actual_thread_name));

  MakeFakeTaskRenamePerfEvent(kTid, kShortComm, kTimestampNs)->Accept(&visitor_);
  ThreadName expected_thread_name = MakeThreadName(-1, kTid, kShortComm, kTimestampNs);
  EXPECT_THAT(actual_thread_name, ThreadNameEq(expected_thread_name));
}

TEST_F(SwitchesStatesNamesVisitorTest, TaskRenameOfTidWithForkPerfEventCausesThreadNameWithPid) {
  MakeFakeForkPerfEvent(kPid, kTid)->Accept(&visitor_);

  ThreadName actual_thread_name;
  EXPECT_CALL(mock_listener_, OnThreadName).Times(1).WillOnce(SaveArg<0>(&actual_thread_name));

  MakeFakeTaskNewtaskPerfEvent(kTid, kShortComm, kTimestampNs)->Accept(&visitor_);
  ThreadName expected_thread_name = MakeThreadName(kPid, kTid, kShortComm, kTimestampNs);
  EXPECT_THAT(actual_thread_name, ThreadNameEq(expected_thread_name));
}

TEST_F(SwitchesStatesNamesVisitorTest, SchedSwitchesAreIgnoredWithoutSetProduceSchedulingSlices) {
  EXPECT_CALL(mock_listener_, OnSchedulingSlice).Times(0);

  MakeFakeSchedSwitchPerfEvent(kCpu, kPrevTid, kPrevTid, kRunnableStateMask, kTid, kInTimestampNs)
      ->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu, kPid, kTid, kRunnableStateMask, kNextTid, kOutTimestampNs)
      ->Accept(&visitor_);
}

TEST_F(SwitchesStatesNamesVisitorTest, SchedSwitchesWithZeroTidAreIgnored) {
  constexpr pid_t kZeroTid = 0;

  visitor_.SetProduceSchedulingSlices(true);

  EXPECT_CALL(mock_listener_, OnSchedulingSlice).Times(0);

  MakeFakeSchedSwitchPerfEvent(kCpu, kPrevTid, kPrevTid, kRunnableStateMask, kZeroTid,
                               kInTimestampNs)
      ->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu, kZeroTid, kZeroTid, kRunnableStateMask, kNextTid,
                               kOutTimestampNs)
      ->Accept(&visitor_);
}

TEST_F(SwitchesStatesNamesVisitorTest,
       SchedSwitchesOfUnknownPidWithMinusOnePidCauseSchedulingSliceWithMinusOnePid) {
  visitor_.SetProduceSchedulingSlices(true);

  SchedulingSlice actual_scheduling_slice;
  EXPECT_CALL(mock_listener_, OnSchedulingSlice)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_scheduling_slice));

  MakeFakeSchedSwitchPerfEvent(kCpu, kPrevTid, kPrevTid, kRunnableStateMask, kTid, kInTimestampNs)
      ->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu, kMinusOnePid, kTid, kRunnableStateMask, kNextTid,
                               kOutTimestampNs)
      ->Accept(&visitor_);

  SchedulingSlice expected_scheduling_slice =
      MakeSchedulingSlice(-1, kTid, kCpu, kOutTimestampNs - kInTimestampNs, kOutTimestampNs);
  EXPECT_THAT(actual_scheduling_slice, SchedulingSliceEq(expected_scheduling_slice));
}

TEST_F(SwitchesStatesNamesVisitorTest,
       SchedSwitchesWithMinusOnePidOfTidWithExitPerfEventCauseSchedulingSliceWithPid) {
  visitor_.SetProduceSchedulingSlices(true);

  SchedulingSlice actual_scheduling_slice;
  EXPECT_CALL(mock_listener_, OnSchedulingSlice)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_scheduling_slice));

  MakeFakeSchedSwitchPerfEvent(kCpu, kPrevTid, kPrevTid, kRunnableStateMask, kTid, kInTimestampNs)
      ->Accept(&visitor_);
  MakeFakeExitPerfEvent(kPid, kTid)->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu, kMinusOnePid, kTid, kRunnableStateMask, kNextTid,
                               kOutTimestampNs)
      ->Accept(&visitor_);

  SchedulingSlice expected_scheduling_slice =
      MakeSchedulingSlice(kPid, kTid, kCpu, kOutTimestampNs - kInTimestampNs, kOutTimestampNs);
  EXPECT_THAT(actual_scheduling_slice, SchedulingSliceEq(expected_scheduling_slice));
}

TEST_F(SwitchesStatesNamesVisitorTest,
       SchedSwitchesOfUnknownPidButWithPidCauseSchedulingSliceWithPid) {
  visitor_.SetProduceSchedulingSlices(true);

  SchedulingSlice actual_scheduling_slice;
  EXPECT_CALL(mock_listener_, OnSchedulingSlice)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_scheduling_slice));

  MakeFakeSchedSwitchPerfEvent(kCpu, kPrevTid, kPrevTid, kRunnableStateMask, kTid, kInTimestampNs)
      ->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu, kPid, kTid, kRunnableStateMask, kNextTid, kOutTimestampNs)
      ->Accept(&visitor_);

  SchedulingSlice expected_scheduling_slice =
      MakeSchedulingSlice(kPid, kTid, kCpu, kOutTimestampNs - kInTimestampNs, kOutTimestampNs);
  EXPECT_THAT(actual_scheduling_slice, SchedulingSliceEq(expected_scheduling_slice));
}

TEST_F(SwitchesStatesNamesVisitorTest,
       SchedSwitchesWithPidOfTidWithInitialTidToPidAssociationCausesSchedulingSliceWithPid) {
  visitor_.SetProduceSchedulingSlices(true);

  visitor_.ProcessInitialTidToPidAssociation(kTid, kPid);

  SchedulingSlice actual_scheduling_slice;
  EXPECT_CALL(mock_listener_, OnSchedulingSlice)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_scheduling_slice));

  MakeFakeSchedSwitchPerfEvent(kCpu, kPrevTid, kPrevTid, kRunnableStateMask, kTid, kInTimestampNs)
      ->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu, kPid, kTid, kRunnableStateMask, kNextTid, kOutTimestampNs)
      ->Accept(&visitor_);

  SchedulingSlice expected_scheduling_slice =
      MakeSchedulingSlice(kPid, kTid, kCpu, kOutTimestampNs - kInTimestampNs, kOutTimestampNs);
  EXPECT_THAT(actual_scheduling_slice, SchedulingSliceEq(expected_scheduling_slice));
}

TEST_F(SwitchesStatesNamesVisitorTest,
       SchedSwitchesWithPidNotMatchingKnownPidCauseNoSchedulingSlices) {
  constexpr pid_t kMismatchingPid = 99;

  visitor_.SetProduceSchedulingSlices(true);

  visitor_.ProcessInitialTidToPidAssociation(kTid, kPid);

  EXPECT_CALL(mock_listener_, OnSchedulingSlice).Times(0);

  MakeFakeSchedSwitchPerfEvent(kCpu, kPrevTid, kPrevTid, kRunnableStateMask, kTid, kInTimestampNs)
      ->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu, kMismatchingPid, kTid, kRunnableStateMask, kNextTid,
                               kOutTimestampNs)
      ->Accept(&visitor_);
}

void SwitchesStatesNamesVisitorTest::ProcessFakeEventsForThreadStateTests() {
  // kTid1
  visitor_.ProcessInitialState(kStartTimestampNs, kTid1, 'D');
  MakeFakeSchedWakeupPerfEvent(kTid1, kWakeTimestampNs1)->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu1, kPrevTid, kPrevTid, kRunnableStateMask, kTid1,
                               kInTimestampNs1)
      ->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu1, kPid, kTid1, kDeadStateMask, kNextTid, kOutTimestampNs1)
      ->Accept(&visitor_);

  // kTid2
  MakeFakeTaskNewtaskPerfEvent(kTid2, kComm2, kNewTimestampNs2)->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu2, kPrevTid, kPrevTid, kRunnableStateMask, kTid2,
                               kInTimestampNs2)
      ->Accept(&visitor_);
  MakeFakeSchedSwitchPerfEvent(kCpu2, kPid, kTid2, kInterruptibleSleepStateMask, kNextTid,
                               kOutTimestampNs2)
      ->Accept(&visitor_);
  MakeFakeSchedWakeupPerfEvent(kTid2, kWakeTimestampNs2)->Accept(&visitor_);

  visitor_.ProcessRemainingOpenStates(kStopTimestampNs);
}

TEST_F(SwitchesStatesNamesVisitorTest, NoThreadStatesWithoutSetThreadStatePidFilter) {
  visitor_.ProcessInitialTidToPidAssociation(kTid1, kPid);
  MakeFakeForkPerfEvent(kPid, kTid2)->Accept(&visitor_);

  EXPECT_CALL(mock_listener_, OnThreadName).Times(1);
  EXPECT_CALL(mock_listener_, OnThreadStateSlice).Times(0);

  ProcessFakeEventsForThreadStateTests();

  EXPECT_EQ(thread_state_counter_, 0);
}

TEST_F(SwitchesStatesNamesVisitorTest, NoThreadStatesOfUnknownPid) {
  visitor_.SetThreadStatePidFilter(kPid);

  EXPECT_CALL(mock_listener_, OnThreadName).Times(1);
  EXPECT_CALL(mock_listener_, OnThreadStateSlice).Times(0);

  ProcessFakeEventsForThreadStateTests();

  EXPECT_EQ(thread_state_counter_, 0);
}

TEST_F(SwitchesStatesNamesVisitorTest, NoThreadStatesOfUninterestingPid) {
  constexpr pid_t kPidFilter = 99;
  visitor_.SetThreadStatePidFilter(kPidFilter);

  visitor_.ProcessInitialTidToPidAssociation(kTid1, kPid);
  MakeFakeForkPerfEvent(kPid, kTid2)->Accept(&visitor_);

  EXPECT_CALL(mock_listener_, OnThreadName).Times(1);
  EXPECT_CALL(mock_listener_, OnThreadStateSlice).Times(0);

  ProcessFakeEventsForThreadStateTests();

  EXPECT_EQ(thread_state_counter_, 0);
}

TEST_F(SwitchesStatesNamesVisitorTest, VariousThreadStateSlicesFromAllPossibleEvents) {
  visitor_.SetThreadStatePidFilter(kPid);

  visitor_.ProcessInitialTidToPidAssociation(kTid1, kPid);
  MakeFakeForkPerfEvent(kPid, kTid2)->Accept(&visitor_);

  EXPECT_CALL(mock_listener_, OnThreadName).Times(1);
  std::vector<ThreadStateSlice> actual_thread_state_slices;
  const auto save_thread_state_slice_arg = [&](ThreadStateSlice actual_thread_state_slice) {
    actual_thread_state_slices.emplace_back(std::move(actual_thread_state_slice));
  };
  EXPECT_CALL(mock_listener_, OnThreadStateSlice)
      .Times(8)
      .WillRepeatedly(save_thread_state_slice_arg);

  ProcessFakeEventsForThreadStateTests();

  EXPECT_EQ(thread_state_counter_, 8);
  EXPECT_THAT(actual_thread_state_slices,
              // UnorderedElementsAre because ProcessRemainingOpenStates could process in any order.
              ::testing::UnorderedElementsAre(
                  ThreadStateSliceEq(MakeThreadStateSlice(
                      kTid1, ThreadStateSlice::kUninterruptibleSleep,
                      kWakeTimestampNs1 - kStartTimestampNs, kWakeTimestampNs1)),
                  ThreadStateSliceEq(MakeThreadStateSlice(kTid1, ThreadStateSlice::kRunnable,
                                                          kInTimestampNs1 - kWakeTimestampNs1,
                                                          kInTimestampNs1)),
                  ThreadStateSliceEq(MakeThreadStateSlice(kTid1, ThreadStateSlice::kRunning,
                                                          kOutTimestampNs1 - kInTimestampNs1,
                                                          kOutTimestampNs1)),
                  ThreadStateSliceEq(MakeThreadStateSlice(kTid1, ThreadStateSlice::kDead,
                                                          kStopTimestampNs - kOutTimestampNs1,
                                                          kStopTimestampNs)),
                  ThreadStateSliceEq(MakeThreadStateSlice(kTid2, ThreadStateSlice::kRunnable,
                                                          kInTimestampNs2 - kNewTimestampNs2,
                                                          kInTimestampNs2)),
                  ThreadStateSliceEq(MakeThreadStateSlice(kTid2, ThreadStateSlice::kRunning,
                                                          kOutTimestampNs2 - kInTimestampNs2,
                                                          kOutTimestampNs2)),
                  ThreadStateSliceEq(MakeThreadStateSlice(
                      kTid2, ThreadStateSlice::kInterruptibleSleep,
                      kWakeTimestampNs2 - kOutTimestampNs2, kWakeTimestampNs2)),
                  ThreadStateSliceEq(MakeThreadStateSlice(kTid2, ThreadStateSlice::kRunnable,
                                                          kStopTimestampNs - kWakeTimestampNs2,
                                                          kStopTimestampNs))));
}

}  // namespace orbit_linux_tracing
