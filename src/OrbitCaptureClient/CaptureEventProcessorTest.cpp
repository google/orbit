// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <string>
#include <vector>

#include "OrbitBase/Result.h"
#include "OrbitCaptureClient/CaptureEventProcessor.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitClientData/Callstack.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/TracepointCustom.h"
#include "OrbitClientData/UserDefinedCaptureData.h"
#include "capture.pb.h"
#include "capture_data.pb.h"
#include "gtest/gtest.h"
#include "tracepoint.pb.h"

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::FunctionInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::ThreadStateSliceInfo;
using orbit_client_protos::TimerInfo;
using orbit_client_protos::TracepointEventInfo;
using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CallstackSample;
using orbit_grpc_protos::CaptureEvent;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::InternedCallstack;
using orbit_grpc_protos::InternedString;
using orbit_grpc_protos::InternedTracepointInfo;
using orbit_grpc_protos::IntrospectionScope;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;
using orbit_grpc_protos::TracepointEvent;
using orbit_grpc_protos::TracepointInfo;

using ::testing::_;
using ::testing::DoAll;
using ::testing::SaveArg;

namespace {

class MockCaptureListener : public CaptureListener {
 public:
  MOCK_METHOD(
      void, OnCaptureStarted,
      (ProcessData&& /*process*/,
       (absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>)/*selected_functions*/,
       TracepointInfoSet /*selected_tracepoints*/,
       UserDefinedCaptureData /*user_defined_capture_data*/),
      (override));
  MOCK_METHOD(void, OnCaptureComplete, (), (override));
  MOCK_METHOD(void, OnCaptureCancelled, (), (override));
  MOCK_METHOD(void, OnCaptureFailed, (ErrorMessage), (override));
  MOCK_METHOD(void, OnTimer, (const TimerInfo&), (override));
  MOCK_METHOD(void, OnKeyAndString, (uint64_t /*key*/, std::string), (override));
  MOCK_METHOD(void, OnUniqueCallStack, (CallStack), (override));
  MOCK_METHOD(void, OnCallstackEvent, (CallstackEvent), (override));
  MOCK_METHOD(void, OnThreadName, (int32_t /*thread_id*/, std::string /*thread_name*/), (override));
  MOCK_METHOD(void, OnThreadStateSlice, (ThreadStateSliceInfo), (override));
  MOCK_METHOD(void, OnAddressInfo, (LinuxAddressInfo), (override));
  MOCK_METHOD(void, OnUniqueTracepointInfo, (uint64_t /*key*/, TracepointInfo /*tracepoint_info*/),
              (override));
  MOCK_METHOD(void, OnTracepointEvent, (TracepointEventInfo), (override));
};

TEST(CaptureEventProcessor, CanHandleSchedulingSlices) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event;
  SchedulingSlice* scheduling_slice = event.mutable_scheduling_slice();
  scheduling_slice->set_core(2);
  scheduling_slice->set_pid(42);
  scheduling_slice->set_tid(24);
  scheduling_slice->set_in_timestamp_ns(3);
  scheduling_slice->set_out_timestamp_ns(100);

  TimerInfo actual_timer;
  EXPECT_CALL(listener, OnTimer).Times(1).WillOnce(SaveArg<0>(&actual_timer));

  event_processor.ProcessEvent(event);

  EXPECT_EQ(actual_timer.start(), scheduling_slice->in_timestamp_ns());
  EXPECT_EQ(actual_timer.end(), scheduling_slice->out_timestamp_ns());
  EXPECT_EQ(actual_timer.process_id(), scheduling_slice->pid());
  EXPECT_EQ(actual_timer.thread_id(), scheduling_slice->tid());
  EXPECT_EQ(actual_timer.processor(), scheduling_slice->core());
  EXPECT_EQ(actual_timer.type(), TimerInfo::kCoreActivity);
}

static CallstackSample* AddAndInitializeCallstackSample(CaptureEvent& event) {
  CallstackSample* callstack_sample = event.mutable_callstack_sample();
  callstack_sample->set_pid(1);
  callstack_sample->set_tid(3);
  Callstack* callstack = callstack_sample->mutable_callstack();
  callstack->add_pcs(14);
  callstack->add_pcs(15);
  return callstack_sample;
}

static void ExpectCallstackSamplesEqual(const CallstackEvent& actual_callstack_event,
                                        const CallStack& actual_call_stack,
                                        const CallstackSample* expected_callstack_sample,
                                        const Callstack* expected_callstack) {
  EXPECT_EQ(actual_callstack_event.time(), expected_callstack_sample->timestamp_ns());
  EXPECT_EQ(actual_callstack_event.thread_id(), expected_callstack_sample->tid());
  EXPECT_EQ(actual_callstack_event.callstack_hash(), actual_call_stack.GetHash());
  ASSERT_EQ(actual_call_stack.GetFramesCount(), expected_callstack->pcs_size());
  for (size_t i = 0; i < actual_call_stack.GetFramesCount(); ++i) {
    EXPECT_EQ(actual_call_stack.GetFrame(i), expected_callstack->pcs(i));
  }
}

TEST(CaptureEventProcessor, CanHandleOneCallstackSample) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event;
  CallstackSample* callstack_sample = AddAndInitializeCallstackSample(event);
  callstack_sample->set_timestamp_ns(100);

  CallStack actual_call_stack;
  EXPECT_CALL(listener, OnUniqueCallStack).Times(1).WillOnce(SaveArg<0>(&actual_call_stack));
  CallstackEvent actual_callstack_event;
  EXPECT_CALL(listener, OnCallstackEvent).Times(1).WillOnce(SaveArg<0>(&actual_callstack_event));

  event_processor.ProcessEvent(event);

  ExpectCallstackSamplesEqual(actual_callstack_event, actual_call_stack, callstack_sample,
                              &callstack_sample->callstack());
}

TEST(CaptureEventProcessor, WillOnlyHandleUniqueCallstacksOnce) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);
  std::vector<CaptureEvent> events;

  CaptureEvent event_1;
  CallstackSample* callstack_sample_1 = AddAndInitializeCallstackSample(event_1);
  callstack_sample_1->set_timestamp_ns(100);

  CaptureEvent event_2;
  CallstackSample* callstack_sample_2 = AddAndInitializeCallstackSample(event_2);
  callstack_sample_2->set_timestamp_ns(200);

  CallStack actual_call_stack;
  EXPECT_CALL(listener, OnUniqueCallStack).Times(1).WillOnce(SaveArg<0>(&actual_call_stack));
  CallstackEvent actual_call_stack_event_1;
  CallstackEvent actual_call_stack_event_2;
  EXPECT_CALL(listener, OnCallstackEvent)
      .Times(2)
      .WillOnce(SaveArg<0>(&actual_call_stack_event_1))
      .WillOnce(SaveArg<0>(&actual_call_stack_event_2));

  event_processor.ProcessEvent(event_1);
  event_processor.ProcessEvent(event_2);

  ExpectCallstackSamplesEqual(actual_call_stack_event_1, actual_call_stack, callstack_sample_1,
                              &callstack_sample_1->callstack());
  ExpectCallstackSamplesEqual(actual_call_stack_event_2, actual_call_stack, callstack_sample_2,
                              &callstack_sample_2->callstack());
}

TEST(CaptureEventProcessor, CanHandleInternedCallstackSamples) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent interned_callstack_event;
  InternedCallstack* interned_callstack = interned_callstack_event.mutable_interned_callstack();
  interned_callstack->set_key(2);
  Callstack* callstack_intern = interned_callstack->mutable_intern();
  callstack_intern->add_pcs(15);
  callstack_intern->add_pcs(16);

  CaptureEvent callstack_event;
  CallstackSample* callstack_sample = callstack_event.mutable_callstack_sample();
  callstack_sample->set_pid(1);
  callstack_sample->set_tid(3);
  callstack_sample->set_callstack_key(interned_callstack->key());
  callstack_sample->set_timestamp_ns(100);

  CallStack actual_call_stack;
  EXPECT_CALL(listener, OnUniqueCallStack).Times(1).WillOnce(SaveArg<0>(&actual_call_stack));
  CallstackEvent actual_call_stack_event;
  EXPECT_CALL(listener, OnCallstackEvent).Times(1).WillOnce(SaveArg<0>(&actual_call_stack_event));

  event_processor.ProcessEvent(interned_callstack_event);
  event_processor.ProcessEvent(callstack_event);

  ExpectCallstackSamplesEqual(actual_call_stack_event, actual_call_stack, callstack_sample,
                              callstack_intern);
}

TEST(CaptureEventProcessor, CanHandleFunctionCalls) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event;
  FunctionCall* function_call = event.mutable_function_call();
  function_call->set_pid(42);
  function_call->set_tid(24);
  function_call->set_absolute_address(123);
  function_call->set_begin_timestamp_ns(3);
  function_call->set_end_timestamp_ns(100);
  function_call->set_depth(3);
  function_call->set_return_value(16);
  function_call->add_registers(4);
  function_call->add_registers(5);

  TimerInfo actual_timer;
  EXPECT_CALL(listener, OnTimer).Times(1).WillOnce(SaveArg<0>(&actual_timer));

  event_processor.ProcessEvent(event);

  EXPECT_EQ(actual_timer.process_id(), function_call->pid());
  EXPECT_EQ(actual_timer.thread_id(), function_call->tid());
  EXPECT_EQ(actual_timer.function_address(), function_call->absolute_address());
  EXPECT_EQ(actual_timer.start(), function_call->begin_timestamp_ns());
  EXPECT_EQ(actual_timer.end(), function_call->end_timestamp_ns());
  EXPECT_EQ(actual_timer.depth(), function_call->depth());
  EXPECT_EQ(actual_timer.user_data_key(), function_call->return_value());
  ASSERT_EQ(actual_timer.registers_size(), function_call->registers_size());
  for (int i = 0; i < actual_timer.registers_size(); ++i) {
    EXPECT_EQ(actual_timer.registers(i), function_call->registers(i));
  }
  EXPECT_EQ(actual_timer.type(), TimerInfo::kNone);
}

TEST(CaptureEventProcessor, CanHandleIntrospectionScopes) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event;
  IntrospectionScope* introspection_scope = event.mutable_introspection_scope();
  introspection_scope->set_pid(42);
  introspection_scope->set_tid(24);
  introspection_scope->set_begin_timestamp_ns(3);
  introspection_scope->set_end_timestamp_ns(100);
  introspection_scope->set_depth(3);
  introspection_scope->add_registers(4);
  introspection_scope->add_registers(5);

  TimerInfo actual_timer;
  EXPECT_CALL(listener, OnTimer).Times(1).WillOnce(SaveArg<0>(&actual_timer));

  event_processor.ProcessEvent(event);

  EXPECT_EQ(actual_timer.process_id(), introspection_scope->pid());
  EXPECT_EQ(actual_timer.thread_id(), introspection_scope->tid());
  EXPECT_EQ(actual_timer.start(), introspection_scope->begin_timestamp_ns());
  EXPECT_EQ(actual_timer.end(), introspection_scope->end_timestamp_ns());
  EXPECT_EQ(actual_timer.depth(), introspection_scope->depth());
  ASSERT_EQ(actual_timer.registers_size(), introspection_scope->registers_size());
  for (int i = 0; i < actual_timer.registers_size(); ++i) {
    EXPECT_EQ(actual_timer.registers(i), introspection_scope->registers(i));
  }
  EXPECT_EQ(actual_timer.type(), TimerInfo::kIntrospection);
}

TEST(CaptureEventProcessor, CanHandleThreadNames) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event;
  ThreadName* thread_name = event.mutable_thread_name();
  thread_name->set_pid(42);
  thread_name->set_tid(24);
  thread_name->set_name("Thread");
  thread_name->set_timestamp_ns(100);

  EXPECT_CALL(listener, OnThreadName(thread_name->tid(), thread_name->name())).Times(1);

  event_processor.ProcessEvent(event);
}

TEST(CaptureEventProcessor, CanHandleAddressInfos) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event;
  AddressInfo* address_info = event.mutable_address_info();
  address_info->set_absolute_address(42);
  address_info->set_function_name("Function");
  address_info->set_offset_in_function(14);
  address_info->set_map_name("module");

  LinuxAddressInfo actual_address_info;
  EXPECT_CALL(listener, OnAddressInfo).Times(1).WillOnce(SaveArg<0>(&actual_address_info));

  event_processor.ProcessEvent(event);

  EXPECT_EQ(actual_address_info.absolute_address(), address_info->absolute_address());
  EXPECT_EQ(actual_address_info.function_name(), address_info->function_name());
  EXPECT_EQ(actual_address_info.offset_in_function(), address_info->offset_in_function());
  EXPECT_EQ(actual_address_info.module_path(), address_info->map_name());
}

TEST(CaptureEventProcessor, CanHandleAddressInfosWithInternedStrings) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent interned_function_name_event;
  InternedString* interned_function_name = interned_function_name_event.mutable_interned_string();
  interned_function_name->set_key(1);
  interned_function_name->set_intern("function");

  CaptureEvent interned_map_name_event;
  InternedString* interned_map_name = interned_map_name_event.mutable_interned_string();
  interned_map_name->set_key(2);
  interned_map_name->set_intern("module");

  CaptureEvent address_info_event;
  AddressInfo* address_info = address_info_event.mutable_address_info();
  address_info->set_absolute_address(42);
  address_info->set_function_name_key(interned_function_name->key());
  address_info->set_offset_in_function(14);
  address_info->set_map_name_key(interned_map_name->key());

  LinuxAddressInfo actual_address_info;
  EXPECT_CALL(listener, OnAddressInfo).Times(1).WillOnce(SaveArg<0>(&actual_address_info));

  event_processor.ProcessEvent(interned_function_name_event);
  event_processor.ProcessEvent(interned_map_name_event);
  event_processor.ProcessEvent(address_info_event);

  EXPECT_EQ(actual_address_info.absolute_address(), address_info->absolute_address());
  EXPECT_EQ(actual_address_info.function_name(), interned_function_name->intern());
  EXPECT_EQ(actual_address_info.offset_in_function(), address_info->offset_in_function());
  EXPECT_EQ(actual_address_info.module_path(), interned_map_name->intern());
}

// TODO(b/174145461): Enable the test as soon as the bug is fixed.
TEST(DISABLED_CaptureEventProcessor, CanHandleOneTracepointEvent) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event;
  TracepointEvent* tracepoint_event = event.mutable_tracepoint_event();
  tracepoint_event->set_pid(1);
  tracepoint_event->set_tid(3);
  tracepoint_event->set_time(100);
  tracepoint_event->set_cpu(2);
  TracepointInfo* tracepoint = tracepoint_event->mutable_tracepoint_info();
  tracepoint->set_category("cat");
  tracepoint->set_name("name");

  uint64_t actual_key;
  TracepointInfo actual_tracepoint_info;
  EXPECT_CALL(listener, OnUniqueTracepointInfo)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_key), SaveArg<1>(&actual_tracepoint_info)));
  TracepointEventInfo actual_tracepoint_event;
  EXPECT_CALL(listener, OnTracepointEvent).Times(1).WillOnce(SaveArg<0>(&actual_tracepoint_event));

  event_processor.ProcessEvent(event);

  EXPECT_EQ(actual_key, actual_tracepoint_event.tracepoint_info_key());
  EXPECT_EQ(actual_tracepoint_info.category(), tracepoint->category());
  EXPECT_EQ(actual_tracepoint_info.name(), tracepoint->name());
  EXPECT_EQ(actual_tracepoint_event.pid(), tracepoint_event->pid());
  EXPECT_EQ(actual_tracepoint_event.tid(), tracepoint_event->tid());
  EXPECT_EQ(actual_tracepoint_event.time(), tracepoint_event->time());
  EXPECT_EQ(actual_tracepoint_event.cpu(), tracepoint_event->cpu());
}

// TODO(b/174145461): Enable the test as soon as the bug is fixed.
TEST(DISABLED_CaptureEventProcessor, WillOnlyHandleUniqueTracepointEventsOnce) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event_1;
  TracepointEvent* tracepoint_event_1 = event_1.mutable_tracepoint_event();
  tracepoint_event_1->set_pid(1);
  tracepoint_event_1->set_tid(3);
  tracepoint_event_1->set_time(100);
  tracepoint_event_1->set_cpu(2);
  TracepointInfo* tracepoint_1 = tracepoint_event_1->mutable_tracepoint_info();
  tracepoint_1->set_category("cat");
  tracepoint_1->set_name("name");

  CaptureEvent event_2;
  TracepointEvent* tracepoint_event_2 = event_2.mutable_tracepoint_event();
  tracepoint_event_2->set_pid(1);
  tracepoint_event_2->set_tid(3);
  tracepoint_event_2->set_time(100);
  tracepoint_event_2->set_cpu(2);
  TracepointInfo* tracepoint_2 = tracepoint_event_1->mutable_tracepoint_info();
  tracepoint_2->set_category("cat");
  tracepoint_2->set_name("name");

  uint64_t actual_key;
  TracepointInfo actual_tracepoint_info;
  EXPECT_CALL(listener, OnUniqueTracepointInfo)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_key), SaveArg<1>(&actual_tracepoint_info)));
  TracepointEventInfo actual_tracepoint_event_1;
  TracepointEventInfo actual_tracepoint_event_2;
  EXPECT_CALL(listener, OnTracepointEvent)
      .Times(2)
      .WillOnce(SaveArg<0>(&actual_tracepoint_event_1))
      .WillOnce(SaveArg<0>(&actual_tracepoint_event_2));

  event_processor.ProcessEvent(event_1);
  event_processor.ProcessEvent(event_2);
}

TEST(CaptureEventProcessor, CanHandleInternedTracepointEvents) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent interned_tracepoint_event;
  InternedTracepointInfo* interned_tracepoint =
      interned_tracepoint_event.mutable_interned_tracepoint_info();
  interned_tracepoint->set_key(2);
  TracepointInfo* tracepoint_intern = interned_tracepoint->mutable_intern();
  tracepoint_intern->set_name("name");
  tracepoint_intern->set_category("category");

  CaptureEvent tracepoint_event;
  TracepointEvent* tracepoint = tracepoint_event.mutable_tracepoint_event();
  tracepoint->set_pid(1);
  tracepoint->set_tid(3);
  tracepoint->set_time(100);
  tracepoint->set_cpu(2);
  tracepoint->set_tracepoint_info_key(interned_tracepoint->key());

  uint64_t actual_key;
  TracepointInfo actual_tracepoint_info;
  EXPECT_CALL(listener, OnUniqueTracepointInfo)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_key), SaveArg<1>(&actual_tracepoint_info)));
  TracepointEventInfo actual_tracepoint_event;
  EXPECT_CALL(listener, OnTracepointEvent).Times(1).WillOnce(SaveArg<0>(&actual_tracepoint_event));

  event_processor.ProcessEvent(interned_tracepoint_event);
  event_processor.ProcessEvent(tracepoint_event);

  EXPECT_EQ(actual_key, actual_tracepoint_event.tracepoint_info_key());
  EXPECT_EQ(actual_tracepoint_event.tracepoint_info_key(), tracepoint->tracepoint_info_key());
  EXPECT_EQ(actual_tracepoint_info.category(), tracepoint_intern->category());
  EXPECT_EQ(actual_tracepoint_info.name(), tracepoint_intern->name());
  EXPECT_EQ(actual_tracepoint_event.pid(), tracepoint->pid());
  EXPECT_EQ(actual_tracepoint_event.tid(), tracepoint->tid());
  EXPECT_EQ(actual_tracepoint_event.time(), tracepoint->time());
  EXPECT_EQ(actual_tracepoint_event.cpu(), tracepoint->cpu());
}

// TODO(b/174210467): Enable the test as soon as the bug is fixed.
TEST(DISABLED_CaptureEventProcessor, CanHandleGpuJobs) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent event;
  GpuJob* gpu_job = event.mutable_gpu_job();
  gpu_job->set_pid(1);
  gpu_job->set_tid(2);
  gpu_job->set_context(3);
  gpu_job->set_seqno(4);
  gpu_job->set_timeline("timeline");
  gpu_job->set_depth(3);
  gpu_job->set_amdgpu_cs_ioctl_time_ns(10);
  gpu_job->set_amdgpu_sched_run_job_time_ns(20);
  gpu_job->set_gpu_hardware_start_time_ns(30);
  gpu_job->set_dma_fence_signaled_time_ns(40);

  uint64_t actual_timeline_key;
  uint64_t actual_sw_queue_key;
  uint64_t actual_hw_queue_key;
  uint64_t actual_hw_execution_key;
  EXPECT_CALL(listener, OnKeyAndString(_, "timeline"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_timeline_key));
  EXPECT_CALL(listener, OnKeyAndString(_, "sw queue"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_sw_queue_key));
  EXPECT_CALL(listener, OnKeyAndString(_, "hw queue"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_hw_queue_key));
  EXPECT_CALL(listener, OnKeyAndString(_, "hw execution"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_hw_execution_key));
  TimerInfo sw_queue_timer;
  TimerInfo hw_queue_timer;
  TimerInfo hw_excecution_timer;
  EXPECT_CALL(listener, OnTimer)
      .Times(3)
      .WillOnce(SaveArg<0>(&sw_queue_timer))
      .WillOnce(SaveArg<0>(&hw_queue_timer))
      .WillOnce(SaveArg<0>(&hw_excecution_timer));

  event_processor.ProcessEvent(event);

  EXPECT_EQ(sw_queue_timer.process_id(), gpu_job->pid());
  EXPECT_EQ(sw_queue_timer.thread_id(), gpu_job->tid());
  EXPECT_EQ(sw_queue_timer.depth(), gpu_job->depth());
  EXPECT_EQ(sw_queue_timer.start(), gpu_job->amdgpu_cs_ioctl_time_ns());
  EXPECT_EQ(sw_queue_timer.end(), gpu_job->amdgpu_sched_run_job_time_ns());
  EXPECT_EQ(sw_queue_timer.type(), TimerInfo::kGpuActivity);
  EXPECT_EQ(sw_queue_timer.timeline_hash(), actual_timeline_key);
  EXPECT_EQ(sw_queue_timer.user_data_key(), actual_sw_queue_key);

  EXPECT_EQ(hw_queue_timer.process_id(), gpu_job->pid());
  EXPECT_EQ(hw_queue_timer.thread_id(), gpu_job->tid());
  EXPECT_EQ(hw_queue_timer.depth(), gpu_job->depth());
  EXPECT_EQ(hw_queue_timer.start(), gpu_job->amdgpu_sched_run_job_time_ns());
  EXPECT_EQ(hw_queue_timer.end(), gpu_job->gpu_hardware_start_time_ns());
  EXPECT_EQ(hw_queue_timer.type(), TimerInfo::kGpuActivity);
  EXPECT_EQ(hw_queue_timer.timeline_hash(), actual_timeline_key);
  EXPECT_EQ(hw_queue_timer.user_data_key(), actual_hw_queue_key);

  EXPECT_EQ(hw_excecution_timer.process_id(), gpu_job->pid());
  EXPECT_EQ(hw_excecution_timer.thread_id(), gpu_job->tid());
  EXPECT_EQ(hw_excecution_timer.depth(), gpu_job->depth());
  EXPECT_EQ(hw_excecution_timer.start(), gpu_job->gpu_hardware_start_time_ns());
  EXPECT_EQ(hw_excecution_timer.end(), gpu_job->dma_fence_signaled_time_ns());
  EXPECT_EQ(hw_excecution_timer.type(), TimerInfo::kGpuActivity);
  EXPECT_EQ(hw_excecution_timer.timeline_hash(), actual_timeline_key);
  EXPECT_EQ(hw_excecution_timer.user_data_key(), actual_hw_execution_key);
}

TEST(CaptureEventProcessor, CanHandleThreadStateSlices) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  CaptureEvent running_event;
  ThreadStateSlice* running_thread_state_slice = running_event.mutable_thread_state_slice();
  running_thread_state_slice->set_begin_timestamp_ns(100);
  running_thread_state_slice->set_end_timestamp_ns(200);
  running_thread_state_slice->set_pid(14);
  running_thread_state_slice->set_tid(24);
  running_thread_state_slice->set_thread_state(ThreadStateSlice::kRunning);

  CaptureEvent runnable_event;
  ThreadStateSlice* runnable_thread_state_slice = runnable_event.mutable_thread_state_slice();
  runnable_thread_state_slice->set_begin_timestamp_ns(100);
  runnable_thread_state_slice->set_end_timestamp_ns(200);
  runnable_thread_state_slice->set_pid(14);
  runnable_thread_state_slice->set_tid(24);
  runnable_thread_state_slice->set_thread_state(ThreadStateSlice::kRunnable);

  CaptureEvent dead_event;
  ThreadStateSlice* dead_thread_state_slice = dead_event.mutable_thread_state_slice();
  dead_thread_state_slice->set_begin_timestamp_ns(100);
  dead_thread_state_slice->set_end_timestamp_ns(200);
  dead_thread_state_slice->set_pid(14);
  dead_thread_state_slice->set_tid(24);
  dead_thread_state_slice->set_thread_state(ThreadStateSlice::kDead);

  ThreadStateSliceInfo actual_running_thread_state_slice_info;
  ThreadStateSliceInfo actual_runnable_thread_state_slice_info;
  ThreadStateSliceInfo actual_dead_thread_state_slice_info;
  EXPECT_CALL(listener, OnThreadStateSlice)
      .Times(3)
      .WillOnce(SaveArg<0>(&actual_running_thread_state_slice_info))
      .WillOnce(SaveArg<0>(&actual_runnable_thread_state_slice_info))
      .WillOnce(SaveArg<0>(&actual_dead_thread_state_slice_info));

  event_processor.ProcessEvent(running_event);
  event_processor.ProcessEvent(runnable_event);
  event_processor.ProcessEvent(dead_event);

  EXPECT_EQ(actual_dead_thread_state_slice_info.begin_timestamp_ns(),
            dead_thread_state_slice->begin_timestamp_ns());
  EXPECT_EQ(actual_dead_thread_state_slice_info.end_timestamp_ns(),
            dead_thread_state_slice->end_timestamp_ns());
  EXPECT_EQ(actual_dead_thread_state_slice_info.tid(), dead_thread_state_slice->tid());
  EXPECT_EQ(actual_dead_thread_state_slice_info.thread_state(), ThreadStateSliceInfo::kDead);
}

TEST(CaptureEventProcessor, CanHandleMultipleEvents) {
  MockCaptureListener listener;
  CaptureEventProcessor event_processor(&listener);

  std::vector<CaptureEvent> events;
  CaptureEvent event_1;
  ThreadName* thread_name = event_1.mutable_thread_name();
  thread_name->set_pid(42);
  thread_name->set_tid(24);
  thread_name->set_name("Thread");
  thread_name->set_timestamp_ns(100);
  events.push_back(event_1);

  EXPECT_CALL(listener, OnThreadName(thread_name->tid(), thread_name->name())).Times(1);

  CaptureEvent event_2;
  AddressInfo* address_info = event_2.mutable_address_info();
  address_info->set_absolute_address(42);
  address_info->set_function_name("Function");
  address_info->set_offset_in_function(14);
  address_info->set_map_name("module");
  events.push_back(event_2);

  LinuxAddressInfo actual_address_info;
  EXPECT_CALL(listener, OnAddressInfo).Times(1).WillOnce(SaveArg<0>(&actual_address_info));

  event_processor.ProcessEvents(events);

  EXPECT_EQ(actual_address_info.absolute_address(), address_info->absolute_address());
  EXPECT_EQ(actual_address_info.function_name(), address_info->function_name());
  EXPECT_EQ(actual_address_info.offset_in_function(), address_info->offset_in_function());
  EXPECT_EQ(actual_address_info.module_path(), address_info->map_name());
}

}  // namespace