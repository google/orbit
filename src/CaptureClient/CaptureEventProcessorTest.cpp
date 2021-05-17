// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <string>
#include <vector>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureClient/CaptureListener.h"
#include "ClientData/TracepointCustom.h"
#include "capture.pb.h"
#include "capture_data.pb.h"
#include "tracepoint.pb.h"

namespace orbit_capture_client {

using orbit_client_protos::CallstackEvent;
using orbit_client_protos::CallstackInfo;
using orbit_client_protos::LinuxAddressInfo;
using orbit_client_protos::ThreadStateSliceInfo;
using orbit_client_protos::TimerInfo;
using orbit_client_protos::TracepointEventInfo;

using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CallstackSample;
using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::ClientCaptureEvent;
using orbit_grpc_protos::Color;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::GpuCommandBuffer;
using orbit_grpc_protos::GpuDebugMarker;
using orbit_grpc_protos::GpuDebugMarkerBeginInfo;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::GpuQueueSubmission;
using orbit_grpc_protos::GpuQueueSubmissionMetaInfo;
using orbit_grpc_protos::GpuSubmitInfo;
using orbit_grpc_protos::InternedCallstack;
using orbit_grpc_protos::InternedString;
using orbit_grpc_protos::InternedTracepointInfo;
using orbit_grpc_protos::IntrospectionScope;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;
using orbit_grpc_protos::TracepointEvent;
using orbit_grpc_protos::TracepointInfo;

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;

namespace {

class MockCaptureListener : public CaptureListener {
 public:
  MOCK_METHOD(void, OnCaptureStarted, (const CaptureStarted&, absl::flat_hash_set<uint64_t>),
              (override));
  MOCK_METHOD(void, OnCaptureFinished, (const CaptureFinished&), (override));
  MOCK_METHOD(void, OnTimer, (const TimerInfo&), (override));
  MOCK_METHOD(void, OnSystemMemoryUsage, (const SystemMemoryUsage&), (override));
  MOCK_METHOD(void, OnKeyAndString, (uint64_t /*key*/, std::string), (override));
  MOCK_METHOD(void, OnUniqueCallstack, (uint64_t /*callstack_id*/, CallstackInfo /*callstack*/),
              (override));
  MOCK_METHOD(void, OnCallstackEvent, (CallstackEvent), (override));
  MOCK_METHOD(void, OnThreadName, (int32_t /*thread_id*/, std::string /*thread_name*/), (override));
  MOCK_METHOD(void, OnThreadStateSlice, (ThreadStateSliceInfo), (override));
  MOCK_METHOD(void, OnAddressInfo, (LinuxAddressInfo), (override));
  MOCK_METHOD(void, OnUniqueTracepointInfo, (uint64_t /*key*/, TracepointInfo /*tracepoint_info*/),
              (override));
  MOCK_METHOD(void, OnTracepointEvent, (TracepointEventInfo), (override));
  MOCK_METHOD(void, OnModuleUpdate, (uint64_t /*timestamp_ns*/, ModuleInfo /*module_info*/),
              (override));
  MOCK_METHOD(void, OnModulesSnapshot,
              (uint64_t /*timestamp_ns*/, std::vector<ModuleInfo> /*module_infos*/), (override));
};

}  // namespace

TEST(CaptureEventProcessor, CanHandleSchedulingSlices) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent event;
  SchedulingSlice* scheduling_slice = event.mutable_scheduling_slice();
  scheduling_slice->set_core(2);
  scheduling_slice->set_pid(42);
  scheduling_slice->set_tid(24);
  scheduling_slice->set_duration_ns(97);
  scheduling_slice->set_out_timestamp_ns(100);

  TimerInfo actual_timer;
  EXPECT_CALL(listener, OnTimer).Times(1).WillOnce(SaveArg<0>(&actual_timer));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_timer.start(),
            scheduling_slice->out_timestamp_ns() - scheduling_slice->duration_ns());
  EXPECT_EQ(actual_timer.end(), scheduling_slice->out_timestamp_ns());
  EXPECT_EQ(actual_timer.process_id(), scheduling_slice->pid());
  EXPECT_EQ(actual_timer.thread_id(), scheduling_slice->tid());
  EXPECT_EQ(actual_timer.processor(), scheduling_slice->core());
  EXPECT_EQ(actual_timer.type(), TimerInfo::kCoreActivity);
}

static InternedCallstack* AddAndInitializeInternedCallstack(ClientCaptureEvent& event) {
  InternedCallstack* interned_callstack = event.mutable_interned_callstack();
  interned_callstack->set_key(1);
  Callstack* callstack = interned_callstack->mutable_intern();
  callstack->add_pcs(14);
  callstack->add_pcs(15);
  callstack->set_type(Callstack::kComplete);
  return interned_callstack;
}

static CallstackSample* AddAndInitializeCallstackSample(ClientCaptureEvent& event) {
  CallstackSample* callstack_sample = event.mutable_callstack_sample();
  callstack_sample->set_pid(1);
  callstack_sample->set_tid(3);
  callstack_sample->set_callstack_id(1);
  return callstack_sample;
}

static void ExpectCallstackSamplesEqual(const CallstackEvent& actual_callstack_event,
                                        uint64_t actual_callstack_id,
                                        const CallstackInfo& actual_callstack,
                                        const CallstackSample* expected_callstack_sample,
                                        const Callstack* expected_callstack) {
  EXPECT_EQ(actual_callstack_event.time(), expected_callstack_sample->timestamp_ns());
  EXPECT_EQ(actual_callstack_event.thread_id(), expected_callstack_sample->tid());
  EXPECT_EQ(actual_callstack_event.callstack_id(), actual_callstack_id);
  ASSERT_EQ(actual_callstack.frames_size(), expected_callstack->pcs_size());
  for (int i = 0; i < actual_callstack.frames_size(); ++i) {
    EXPECT_EQ(actual_callstack.frames(i), expected_callstack->pcs(i));
  }

  switch (expected_callstack->type()) {
    case Callstack::kComplete:
      EXPECT_EQ(actual_callstack.type(), CallstackInfo::kComplete);
      break;
    case Callstack::kDwarfUnwindingError:
      EXPECT_EQ(actual_callstack.type(), CallstackInfo::kDwarfUnwindingError);
      break;
    case Callstack::kFramePointerUnwindingError:
      EXPECT_EQ(actual_callstack.type(), CallstackInfo::kFramePointerUnwindingError);
      break;
    case Callstack::kInUprobes:
      EXPECT_EQ(actual_callstack.type(), CallstackInfo::kInUprobes);
      break;
    case Callstack::kUprobesPatchingFailed:
      EXPECT_EQ(actual_callstack.type(), CallstackInfo::kUprobesPatchingFailed);
      break;
    case orbit_grpc_protos::
        Callstack_CallstackType_Callstack_CallstackType_INT_MIN_SENTINEL_DO_NOT_USE_:
      [[fallthrough]];
    case orbit_grpc_protos::
        Callstack_CallstackType_Callstack_CallstackType_INT_MAX_SENTINEL_DO_NOT_USE_:
      UNREACHABLE();
  }
}

static void CanHandleOneCallstackSampleOfType(Callstack::CallstackType type) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent interned_callstack_event;
  InternedCallstack* interned_callstack =
      AddAndInitializeInternedCallstack(interned_callstack_event);
  interned_callstack->mutable_intern()->set_type(type);

  event_processor->ProcessEvent(interned_callstack_event);

  ClientCaptureEvent event;
  CallstackSample* callstack_sample = AddAndInitializeCallstackSample(event);
  callstack_sample->set_timestamp_ns(100);

  uint64_t actual_callstack_id = 0;
  CallstackInfo actual_callstack;
  EXPECT_CALL(listener, OnUniqueCallstack)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_callstack_id), SaveArg<1>(&actual_callstack)));
  CallstackEvent actual_callstack_event;
  EXPECT_CALL(listener, OnCallstackEvent).Times(1).WillOnce(SaveArg<0>(&actual_callstack_event));

  event_processor->ProcessEvent(event);

  ExpectCallstackSamplesEqual(actual_callstack_event, actual_callstack_id, actual_callstack,
                              callstack_sample, &interned_callstack->intern());
}

TEST(CaptureEventProcessor, CanHandleOneCallstackSample) {
  CanHandleOneCallstackSampleOfType(Callstack::kComplete);
}

TEST(CaptureEventProcessor, CanHandleOneNonCompleteCallstackSample) {
  CanHandleOneCallstackSampleOfType(Callstack::kDwarfUnwindingError);
  CanHandleOneCallstackSampleOfType(Callstack::kFramePointerUnwindingError);
  CanHandleOneCallstackSampleOfType(Callstack::kInUprobes);
  CanHandleOneCallstackSampleOfType(Callstack::kUprobesPatchingFailed);
}

TEST(CaptureEventProcessor, WillOnlyHandleUniqueCallstacksOnce) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});
  std::vector<ClientCaptureEvent> events;

  ClientCaptureEvent event_interned_callstack;
  InternedCallstack* interned_callstack =
      AddAndInitializeInternedCallstack(event_interned_callstack);

  ClientCaptureEvent event_1;
  CallstackSample* callstack_sample_1 = AddAndInitializeCallstackSample(event_1);
  callstack_sample_1->set_timestamp_ns(100);

  ClientCaptureEvent event_2;
  CallstackSample* callstack_sample_2 = AddAndInitializeCallstackSample(event_2);
  callstack_sample_2->set_timestamp_ns(200);

  uint64_t actual_callstack_id = 0;
  CallstackInfo actual_callstack;
  EXPECT_CALL(listener, OnUniqueCallstack)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_callstack_id), SaveArg<1>(&actual_callstack)));
  CallstackEvent actual_call_stack_event_1;
  CallstackEvent actual_call_stack_event_2;
  EXPECT_CALL(listener, OnCallstackEvent)
      .Times(2)
      .WillOnce(SaveArg<0>(&actual_call_stack_event_1))
      .WillOnce(SaveArg<0>(&actual_call_stack_event_2));

  event_processor->ProcessEvent(event_interned_callstack);
  event_processor->ProcessEvent(event_1);
  event_processor->ProcessEvent(event_2);

  ExpectCallstackSamplesEqual(actual_call_stack_event_1, actual_callstack_id, actual_callstack,
                              callstack_sample_1, &interned_callstack->intern());
  ExpectCallstackSamplesEqual(actual_call_stack_event_2, actual_callstack_id, actual_callstack,
                              callstack_sample_2, &interned_callstack->intern());
}

TEST(CaptureEventProcessor, CanHandleInternedCallstackSamples) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent interned_callstack_event;
  InternedCallstack* interned_callstack = interned_callstack_event.mutable_interned_callstack();
  interned_callstack->set_key(2);
  Callstack* callstack_intern = interned_callstack->mutable_intern();
  callstack_intern->add_pcs(15);
  callstack_intern->add_pcs(16);

  ClientCaptureEvent callstack_event;
  CallstackSample* callstack_sample = callstack_event.mutable_callstack_sample();
  callstack_sample->set_pid(1);
  callstack_sample->set_tid(3);
  callstack_sample->set_callstack_id(interned_callstack->key());
  callstack_sample->set_timestamp_ns(100);

  uint64_t actual_callstack_id = 0;
  CallstackInfo actual_callstack;
  EXPECT_CALL(listener, OnUniqueCallstack)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_callstack_id), SaveArg<1>(&actual_callstack)));
  CallstackEvent actual_call_stack_event;
  EXPECT_CALL(listener, OnCallstackEvent).Times(1).WillOnce(SaveArg<0>(&actual_call_stack_event));

  event_processor->ProcessEvent(interned_callstack_event);
  event_processor->ProcessEvent(callstack_event);

  ExpectCallstackSamplesEqual(actual_call_stack_event, actual_callstack_id, actual_callstack,
                              callstack_sample, callstack_intern);
}

TEST(CaptureEventProcessor, CanHandleFunctionCalls) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent event;
  FunctionCall* function_call = event.mutable_function_call();
  function_call->set_pid(42);
  function_call->set_tid(24);
  function_call->set_function_id(123);
  function_call->set_duration_ns(97);
  function_call->set_end_timestamp_ns(100);
  function_call->set_depth(3);
  function_call->set_return_value(16);
  function_call->add_registers(4);
  function_call->add_registers(5);

  TimerInfo actual_timer;
  EXPECT_CALL(listener, OnTimer).Times(1).WillOnce(SaveArg<0>(&actual_timer));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_timer.process_id(), function_call->pid());
  EXPECT_EQ(actual_timer.thread_id(), function_call->tid());
  EXPECT_EQ(actual_timer.function_id(), function_call->function_id());
  EXPECT_EQ(actual_timer.start(), function_call->end_timestamp_ns() - function_call->duration_ns());
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
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent event;
  IntrospectionScope* introspection_scope = event.mutable_introspection_scope();
  introspection_scope->set_pid(42);
  introspection_scope->set_tid(24);
  introspection_scope->set_duration_ns(97);
  introspection_scope->set_end_timestamp_ns(100);
  introspection_scope->set_depth(3);
  introspection_scope->add_registers(4);
  introspection_scope->add_registers(5);

  TimerInfo actual_timer;
  EXPECT_CALL(listener, OnTimer).Times(1).WillOnce(SaveArg<0>(&actual_timer));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_timer.process_id(), introspection_scope->pid());
  EXPECT_EQ(actual_timer.thread_id(), introspection_scope->tid());
  EXPECT_EQ(actual_timer.start(),
            introspection_scope->end_timestamp_ns() - introspection_scope->duration_ns());
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
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent event;
  ThreadName* thread_name = event.mutable_thread_name();
  thread_name->set_pid(42);
  thread_name->set_tid(24);
  thread_name->set_name("Thread");
  thread_name->set_timestamp_ns(100);

  EXPECT_CALL(listener, OnThreadName(thread_name->tid(), thread_name->name())).Times(1);

  event_processor->ProcessEvent(event);
}

static ClientCaptureEvent CreateInternedStringEvent(uint64_t key, std::string str) {
  ClientCaptureEvent capture_event;
  InternedString* interned_string = capture_event.mutable_interned_string();
  interned_string->set_key(key);
  interned_string->set_intern(std::move(str));
  return capture_event;
}

TEST(CaptureEventProcessor, CanHandleAddressInfosWithInternedStrings) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  constexpr uint64_t kFunctionNameKey = 1;
  ClientCaptureEvent interned_function_name_event =
      CreateInternedStringEvent(kFunctionNameKey, "function");

  constexpr uint64_t kModuleNameKey = 2;
  ClientCaptureEvent interned_map_name_event = CreateInternedStringEvent(kModuleNameKey, "module");

  ClientCaptureEvent address_info_event;
  AddressInfo* address_info = address_info_event.mutable_address_info();
  address_info->set_absolute_address(42);
  address_info->set_function_name_key(kFunctionNameKey);
  address_info->set_offset_in_function(14);
  address_info->set_module_name_key(kModuleNameKey);

  EXPECT_CALL(listener, OnKeyAndString(kModuleNameKey, "module")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(kFunctionNameKey, "function")).Times(1);

  LinuxAddressInfo actual_address_info;
  EXPECT_CALL(listener, OnAddressInfo).Times(1).WillOnce(SaveArg<0>(&actual_address_info));

  event_processor->ProcessEvent(interned_function_name_event);
  event_processor->ProcessEvent(interned_map_name_event);
  event_processor->ProcessEvent(address_info_event);

  EXPECT_EQ(actual_address_info.absolute_address(), address_info->absolute_address());
  EXPECT_EQ(actual_address_info.function_name(), "function");
  EXPECT_EQ(actual_address_info.offset_in_function(), address_info->offset_in_function());
  EXPECT_EQ(actual_address_info.module_path(), "module");
}

TEST(CaptureEventProcessor, CanHandleInternedTracepointEvents) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent interned_tracepoint_event;
  InternedTracepointInfo* interned_tracepoint =
      interned_tracepoint_event.mutable_interned_tracepoint_info();
  interned_tracepoint->set_key(2);
  TracepointInfo* tracepoint_intern = interned_tracepoint->mutable_intern();
  tracepoint_intern->set_name("name");
  tracepoint_intern->set_category("category");

  ClientCaptureEvent tracepoint_event;
  TracepointEvent* tracepoint = tracepoint_event.mutable_tracepoint_event();
  tracepoint->set_pid(1);
  tracepoint->set_tid(3);
  tracepoint->set_timestamp_ns(100);
  tracepoint->set_cpu(2);
  tracepoint->set_tracepoint_info_key(interned_tracepoint->key());

  uint64_t actual_key;
  TracepointInfo actual_tracepoint_info;
  EXPECT_CALL(listener, OnUniqueTracepointInfo)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_key), SaveArg<1>(&actual_tracepoint_info)));
  TracepointEventInfo actual_tracepoint_event;
  EXPECT_CALL(listener, OnTracepointEvent).Times(1).WillOnce(SaveArg<0>(&actual_tracepoint_event));

  event_processor->ProcessEvent(interned_tracepoint_event);
  event_processor->ProcessEvent(tracepoint_event);

  EXPECT_EQ(actual_key, actual_tracepoint_event.tracepoint_info_key());
  EXPECT_EQ(actual_tracepoint_event.tracepoint_info_key(), tracepoint->tracepoint_info_key());
  EXPECT_EQ(actual_tracepoint_info.category(), tracepoint_intern->category());
  EXPECT_EQ(actual_tracepoint_info.name(), tracepoint_intern->name());
  EXPECT_EQ(actual_tracepoint_event.pid(), tracepoint->pid());
  EXPECT_EQ(actual_tracepoint_event.tid(), tracepoint->tid());
  EXPECT_EQ(actual_tracepoint_event.time(), tracepoint->timestamp_ns());
  EXPECT_EQ(actual_tracepoint_event.cpu(), tracepoint->cpu());
}

static constexpr int32_t kGpuPid = 1;
static constexpr int32_t kGpuTid = 2;

static GpuJob* CreateGpuJob(ClientCaptureEvent* capture_event, uint64_t timeline_key,
                            uint64_t sw_queue, uint64_t hw_queue, uint64_t hw_execution_begin,
                            uint64_t hw_execution_end) {
  GpuJob* gpu_job = capture_event->mutable_gpu_job();
  gpu_job->set_pid(kGpuPid);
  gpu_job->set_tid(kGpuTid);
  gpu_job->set_context(3);
  gpu_job->set_seqno(4);
  gpu_job->set_timeline_key(timeline_key);
  gpu_job->set_depth(3);
  gpu_job->set_amdgpu_cs_ioctl_time_ns(sw_queue);
  gpu_job->set_amdgpu_sched_run_job_time_ns(hw_queue);
  gpu_job->set_gpu_hardware_start_time_ns(hw_execution_begin);
  gpu_job->set_dma_fence_signaled_time_ns(hw_execution_end);
  return gpu_job;
}

static constexpr uint64_t kTimelineKey = 17;
static constexpr const char* kTimelineString = "timeline";

TEST(CaptureEventProcessor, CanHandleGpuJobs) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent event;
  GpuJob* gpu_job = CreateGpuJob(&event, kTimelineKey, 10, 20, 30, 40);

  uint64_t actual_sw_queue_key;
  uint64_t actual_hw_queue_key;
  uint64_t actual_hw_execution_key;
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

  event_processor->ProcessEvent(event);

  EXPECT_EQ(sw_queue_timer.process_id(), gpu_job->pid());
  EXPECT_EQ(sw_queue_timer.thread_id(), gpu_job->tid());
  EXPECT_EQ(sw_queue_timer.depth(), gpu_job->depth());
  EXPECT_EQ(sw_queue_timer.start(), gpu_job->amdgpu_cs_ioctl_time_ns());
  EXPECT_EQ(sw_queue_timer.end(), gpu_job->amdgpu_sched_run_job_time_ns());
  EXPECT_EQ(sw_queue_timer.type(), TimerInfo::kGpuActivity);
  EXPECT_EQ(sw_queue_timer.timeline_hash(), kTimelineKey);
  EXPECT_EQ(sw_queue_timer.user_data_key(), actual_sw_queue_key);

  EXPECT_EQ(hw_queue_timer.process_id(), gpu_job->pid());
  EXPECT_EQ(hw_queue_timer.thread_id(), gpu_job->tid());
  EXPECT_EQ(hw_queue_timer.depth(), gpu_job->depth());
  EXPECT_EQ(hw_queue_timer.start(), gpu_job->amdgpu_sched_run_job_time_ns());
  EXPECT_EQ(hw_queue_timer.end(), gpu_job->gpu_hardware_start_time_ns());
  EXPECT_EQ(hw_queue_timer.type(), TimerInfo::kGpuActivity);
  EXPECT_EQ(hw_queue_timer.timeline_hash(), kTimelineKey);
  EXPECT_EQ(hw_queue_timer.user_data_key(), actual_hw_queue_key);

  EXPECT_EQ(hw_excecution_timer.process_id(), gpu_job->pid());
  EXPECT_EQ(hw_excecution_timer.thread_id(), gpu_job->tid());
  EXPECT_EQ(hw_excecution_timer.depth(), gpu_job->depth());
  EXPECT_EQ(hw_excecution_timer.start(), gpu_job->gpu_hardware_start_time_ns());
  EXPECT_EQ(hw_excecution_timer.end(), gpu_job->dma_fence_signaled_time_ns());
  EXPECT_EQ(hw_excecution_timer.type(), TimerInfo::kGpuActivity);
  EXPECT_EQ(hw_excecution_timer.timeline_hash(), kTimelineKey);
  EXPECT_EQ(hw_excecution_timer.user_data_key(), actual_hw_execution_key);
}

GpuQueueSubmissionMetaInfo* CreateGpuQueueSubmissionMetaInfo(GpuQueueSubmission* submission,
                                                             uint64_t pre_timestamp,
                                                             uint64_t post_timestamp) {
  GpuQueueSubmissionMetaInfo* meta_info = submission->mutable_meta_info();
  meta_info->set_tid(kGpuTid);
  meta_info->set_pid(kGpuPid);
  meta_info->set_pre_submission_cpu_timestamp(pre_timestamp);
  meta_info->set_post_submission_cpu_timestamp(post_timestamp);
  return meta_info;
}

void AddGpuCommandBufferToGpuSubmitInfo(GpuSubmitInfo* submit_info, uint64_t gpu_begin_timestamp,
                                        uint64_t gpu_end_timestamp) {
  GpuCommandBuffer* command_buffer = submit_info->add_command_buffers();
  command_buffer->set_begin_gpu_timestamp_ns(gpu_begin_timestamp);
  command_buffer->set_end_gpu_timestamp_ns(gpu_end_timestamp);
}

constexpr float kGpuDebugMarkerAlpha = 1.f;
constexpr float kGpuDebugMarkerRed = 0.75f;
constexpr float kGpuDebugMarkerGreen = 0.5f;
constexpr float kGpuDebugMarkerBlue = 0.25f;
constexpr uint32_t kGpuDebugMarkerDepth = 1;

void AddGpuDebugMarkerToGpuQueueSubmission(GpuQueueSubmission* submission,
                                           GpuQueueSubmissionMetaInfo* begin_meta_info,
                                           uint64_t marker_text_key, uint64_t begin_gpu_timestamp,
                                           uint64_t end_gpu_timestamp) {
  GpuDebugMarker* debug_marker = submission->add_completed_markers();
  Color* color = debug_marker->mutable_color();
  color->set_alpha(kGpuDebugMarkerAlpha);
  color->set_red(kGpuDebugMarkerRed);
  color->set_green(kGpuDebugMarkerGreen);
  color->set_blue(kGpuDebugMarkerBlue);
  debug_marker->set_depth(kGpuDebugMarkerDepth);
  debug_marker->set_text_key(marker_text_key);
  debug_marker->set_end_gpu_timestamp_ns(end_gpu_timestamp);
  if (begin_meta_info == nullptr) {
    return;
  }
  GpuDebugMarkerBeginInfo* begin_marker = debug_marker->mutable_begin_marker();
  GpuQueueSubmissionMetaInfo* meta_info_copy = begin_marker->mutable_meta_info();
  meta_info_copy->CopyFrom(*begin_meta_info);
  begin_marker->set_gpu_timestamp_ns(begin_gpu_timestamp);
}

void ExpectCommandBufferTimerEq(const TimerInfo& actual_timer, const GpuJob& gpu_job,
                                uint64_t cpu_begin, uint64_t cpu_end, uint64_t timeline_key,
                                uint64_t command_buffer_key) {
  EXPECT_EQ(actual_timer.thread_id(), gpu_job.tid());
  EXPECT_EQ(actual_timer.process_id(), gpu_job.pid());
  EXPECT_EQ(actual_timer.depth(), gpu_job.depth());
  EXPECT_EQ(actual_timer.start(), cpu_begin);
  EXPECT_EQ(actual_timer.end(), cpu_end);
  EXPECT_EQ(actual_timer.type(), TimerInfo::kGpuCommandBuffer);
  EXPECT_EQ(actual_timer.timeline_hash(), timeline_key);
  EXPECT_EQ(actual_timer.user_data_key(), command_buffer_key);
}

void ExpectDebugMarkerTimerEq(const TimerInfo& actual_timer, uint64_t cpu_begin, uint64_t cpu_end,
                              int32_t thread_id, int32_t process_id, uint32_t depth,
                              uint64_t timeline_key, uint64_t marker_key) {
  EXPECT_EQ(actual_timer.start(), cpu_begin);
  EXPECT_EQ(actual_timer.end(), cpu_end);
  EXPECT_EQ(actual_timer.thread_id(), thread_id);
  EXPECT_EQ(actual_timer.process_id(), process_id);
  EXPECT_EQ(actual_timer.depth(), depth);
  EXPECT_EQ(actual_timer.type(), TimerInfo::kGpuDebugMarker);
  EXPECT_EQ(actual_timer.timeline_hash(), timeline_key);
  EXPECT_EQ(actual_timer.user_data_key(), marker_key);
  EXPECT_EQ(actual_timer.color().alpha(), static_cast<uint32_t>(kGpuDebugMarkerAlpha * 255));
  EXPECT_EQ(actual_timer.color().red(), static_cast<uint32_t>(kGpuDebugMarkerRed * 255));
  EXPECT_EQ(actual_timer.color().green(), static_cast<uint32_t>(kGpuDebugMarkerGreen * 255));
  EXPECT_EQ(actual_timer.color().blue(), static_cast<uint32_t>(kGpuDebugMarkerBlue * 255));
}

TEST(CaptureEventProcessor, CanHandleGpuSubmissionAfterGpuJob) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent timeline_key_and_string =
      CreateInternedStringEvent(kTimelineKey, kTimelineString);

  ClientCaptureEvent gpu_job_event;
  GpuJob* gpu_job = CreateGpuJob(&gpu_job_event, kTimelineKey, 10, 20, 30, 40);

  ClientCaptureEvent marker_string_event = CreateInternedStringEvent(42, "marker");

  ClientCaptureEvent queue_submission_event;
  GpuQueueSubmission* submission = queue_submission_event.mutable_gpu_queue_submission();
  GpuQueueSubmissionMetaInfo* meta_info = CreateGpuQueueSubmissionMetaInfo(submission, 9, 11);

  GpuSubmitInfo* submit_info = submission->add_submit_infos();
  AddGpuCommandBufferToGpuSubmitInfo(submit_info, 115, 119);
  AddGpuCommandBufferToGpuSubmitInfo(submit_info, 120, 124);
  AddGpuDebugMarkerToGpuQueueSubmission(submission, meta_info, 42, 116, 121);
  submission->set_num_begin_markers(1);

  EXPECT_CALL(listener, OnKeyAndString(kTimelineKey, kTimelineString)).Times(1);

  EXPECT_CALL(listener, OnKeyAndString(_, "sw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw execution")).Times(1);

  EXPECT_CALL(listener, OnTimer).Times(3);

  event_processor->ProcessEvent(timeline_key_and_string);
  event_processor->ProcessEvent(gpu_job_event);

  testing::Mock::VerifyAndClearExpectations(&listener);

  EXPECT_CALL(listener, OnKeyAndString(_, "timeline")).Times(0);
  uint64_t actual_marker_key;
  EXPECT_CALL(listener, OnKeyAndString(_, "marker"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_marker_key));

  uint64_t actual_command_buffer_key;
  EXPECT_CALL(listener, OnKeyAndString(_, "command buffer"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_command_buffer_key));

  TimerInfo command_buffer_timer_1;
  TimerInfo command_buffer_timer_2;
  TimerInfo debug_marker_timer;
  EXPECT_CALL(listener, OnTimer)
      .Times(3)
      .WillOnce(SaveArg<0>(&command_buffer_timer_1))
      .WillOnce(SaveArg<0>(&command_buffer_timer_2))
      .WillOnce(SaveArg<0>(&debug_marker_timer));

  event_processor->ProcessEvent(marker_string_event);
  event_processor->ProcessEvent(queue_submission_event);

  ExpectCommandBufferTimerEq(command_buffer_timer_1, *gpu_job, 30, 34, kTimelineKey,
                             actual_command_buffer_key);

  ExpectCommandBufferTimerEq(command_buffer_timer_2, *gpu_job, 35, 39, kTimelineKey,
                             actual_command_buffer_key);

  ExpectDebugMarkerTimerEq(debug_marker_timer, 31, 36, gpu_job->tid(), gpu_job->pid(), 1,
                           kTimelineKey, actual_marker_key);
}

TEST(CaptureEventProcessor, CanHandleGpuSubmissionReceivedBeforeGpuJob) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent timeline_key_and_string =
      CreateInternedStringEvent(kTimelineKey, kTimelineString);

  ClientCaptureEvent gpu_job_event;
  GpuJob* gpu_job = CreateGpuJob(&gpu_job_event, kTimelineKey, 10, 20, 30, 40);

  ClientCaptureEvent marker_string_event = CreateInternedStringEvent(42, "marker");

  ClientCaptureEvent queue_submission_event;
  GpuQueueSubmission* submission = queue_submission_event.mutable_gpu_queue_submission();
  GpuQueueSubmissionMetaInfo* meta_info = CreateGpuQueueSubmissionMetaInfo(submission, 9, 11);

  GpuSubmitInfo* submit_info = submission->add_submit_infos();
  AddGpuCommandBufferToGpuSubmitInfo(submit_info, 115, 119);
  AddGpuCommandBufferToGpuSubmitInfo(submit_info, 120, 124);
  AddGpuDebugMarkerToGpuQueueSubmission(submission, meta_info, 42, 116, 121);
  submission->set_num_begin_markers(1);

  EXPECT_CALL(listener, OnKeyAndString(kTimelineKey, kTimelineString)).Times(1);
  EXPECT_CALL(listener, OnTimer).Times(0);

  event_processor->ProcessEvent(timeline_key_and_string);
  event_processor->ProcessEvent(queue_submission_event);

  testing::Mock::VerifyAndClearExpectations(&listener);

  EXPECT_CALL(listener, OnKeyAndString(_, "sw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw execution")).Times(1);

  EXPECT_CALL(listener, OnKeyAndString(_, "timeline")).Times(0);

  uint64_t actual_command_buffer_key;
  EXPECT_CALL(listener, OnKeyAndString(_, "command buffer"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_command_buffer_key));

  uint64_t actual_marker_key = 0;
  EXPECT_CALL(listener, OnKeyAndString(42, "marker"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_marker_key));

  TimerInfo command_buffer_timer_1;
  TimerInfo command_buffer_timer_2;
  TimerInfo debug_marker_timer;
  EXPECT_CALL(listener, OnTimer)
      .Times(6)
      // The first three timers are from the GpuJob, which we don't test here.
      .WillOnce(Return())
      .WillOnce(Return())
      .WillOnce(Return())
      .WillOnce(SaveArg<0>(&command_buffer_timer_1))
      .WillOnce(SaveArg<0>(&command_buffer_timer_2))
      .WillOnce(SaveArg<0>(&debug_marker_timer));

  event_processor->ProcessEvent(marker_string_event);
  event_processor->ProcessEvent(gpu_job_event);

  ExpectCommandBufferTimerEq(command_buffer_timer_1, *gpu_job, 30, 34, kTimelineKey,
                             actual_command_buffer_key);

  ExpectCommandBufferTimerEq(command_buffer_timer_2, *gpu_job, 35, 39, kTimelineKey,
                             actual_command_buffer_key);

  ExpectDebugMarkerTimerEq(debug_marker_timer, 31, 36, gpu_job->tid(), gpu_job->pid(), 1,
                           kTimelineKey, actual_marker_key);
}

TEST(CaptureEventProcessor, CanHandleGpuDebugMarkersSpreadAcrossSubmissions) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent timeline_string = CreateInternedStringEvent(kTimelineKey, kTimelineString);

  ClientCaptureEvent gpu_job_event_1;
  GpuJob* gpu_job_1 = CreateGpuJob(&gpu_job_event_1, kTimelineKey, 10, 20, 30, 40);
  ClientCaptureEvent gpu_job_event_2;
  GpuJob* gpu_job_2 = CreateGpuJob(&gpu_job_event_2, kTimelineKey, 50, 60, 70, 80);

  ClientCaptureEvent marker_string_event = CreateInternedStringEvent(42, "marker");

  ClientCaptureEvent queue_submission_event_1;
  GpuQueueSubmission* submission_1 = queue_submission_event_1.mutable_gpu_queue_submission();
  GpuQueueSubmissionMetaInfo* meta_info_1 = CreateGpuQueueSubmissionMetaInfo(submission_1, 9, 11);
  GpuSubmitInfo* submit_info_1 = submission_1->add_submit_infos();
  AddGpuCommandBufferToGpuSubmitInfo(submit_info_1, 115, 119);
  AddGpuCommandBufferToGpuSubmitInfo(submit_info_1, 120, 124);
  submission_1->set_num_begin_markers(1);

  ClientCaptureEvent queue_submission_event_2;
  GpuQueueSubmission* submission_2 = queue_submission_event_2.mutable_gpu_queue_submission();
  CreateGpuQueueSubmissionMetaInfo(submission_2, 49, 51);
  GpuSubmitInfo* submit_info_2 = submission_2->add_submit_infos();
  AddGpuCommandBufferToGpuSubmitInfo(submit_info_2, 145, 154);
  AddGpuDebugMarkerToGpuQueueSubmission(submission_2, meta_info_1, 42, 116, 153);

  EXPECT_CALL(listener, OnKeyAndString(kTimelineKey, kTimelineString)).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "sw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw execution")).Times(1);

  EXPECT_CALL(listener, OnTimer).Times(6);

  event_processor->ProcessEvent(timeline_string);
  event_processor->ProcessEvent(gpu_job_event_1);
  event_processor->ProcessEvent(gpu_job_event_2);

  testing::Mock::VerifyAndClearExpectations(&listener);

  uint64_t actual_command_buffer_key;
  EXPECT_CALL(listener, OnKeyAndString(_, "command buffer"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_command_buffer_key));

  TimerInfo command_buffer_timer_1;
  TimerInfo command_buffer_timer_2;
  EXPECT_CALL(listener, OnTimer)
      .Times(2)
      .WillOnce(SaveArg<0>(&command_buffer_timer_1))
      .WillOnce(SaveArg<0>(&command_buffer_timer_2));

  event_processor->ProcessEvent(queue_submission_event_1);

  testing::Mock::VerifyAndClearExpectations(&listener);

  ExpectCommandBufferTimerEq(command_buffer_timer_1, *gpu_job_1, 30, 34, kTimelineKey,
                             actual_command_buffer_key);

  ExpectCommandBufferTimerEq(command_buffer_timer_2, *gpu_job_1, 35, 39, kTimelineKey,
                             actual_command_buffer_key);

  TimerInfo command_buffer_timer_3;
  TimerInfo debug_marker_timer;
  EXPECT_CALL(listener, OnTimer)
      .Times(2)
      .WillOnce(SaveArg<0>(&command_buffer_timer_3))
      .WillOnce(SaveArg<0>(&debug_marker_timer));

  EXPECT_CALL(listener, OnKeyAndString(_, "timeline")).Times(0);
  uint64_t actual_marker_key = 0;
  EXPECT_CALL(listener, OnKeyAndString(_, "marker"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_marker_key));

  event_processor->ProcessEvent(marker_string_event);
  event_processor->ProcessEvent(queue_submission_event_2);
  testing::Mock::VerifyAndClearExpectations(&listener);

  ExpectCommandBufferTimerEq(command_buffer_timer_3, *gpu_job_2, 70, 79, kTimelineKey,
                             actual_command_buffer_key);

  ExpectDebugMarkerTimerEq(debug_marker_timer, 31, 78, gpu_job_2->tid(), gpu_job_2->pid(), 1,
                           kTimelineKey, actual_marker_key);
}

TEST(CaptureEventProcessor, CanHandleGpuDebugMarkersWithNoBeginRecorded) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent timeline_key_and_string =
      CreateInternedStringEvent(kTimelineKey, kTimelineString);
  // The first job that actually contains the begin marker is not recorded.
  ClientCaptureEvent gpu_job_event_2;
  GpuJob* gpu_job_2 = CreateGpuJob(&gpu_job_event_2, kTimelineKey, 50, 60, 70, 80);

  ClientCaptureEvent marker_string_event = CreateInternedStringEvent(42, "marker");

  ClientCaptureEvent queue_submission_event_2;
  GpuQueueSubmission* submission_2 = queue_submission_event_2.mutable_gpu_queue_submission();
  CreateGpuQueueSubmissionMetaInfo(submission_2, 49, 51);
  GpuSubmitInfo* submit_info_2 = submission_2->add_submit_infos();
  AddGpuCommandBufferToGpuSubmitInfo(submit_info_2, 145, 154);
  AddGpuDebugMarkerToGpuQueueSubmission(submission_2, nullptr, 42, 116, 153);

  EXPECT_CALL(listener, OnKeyAndString(kTimelineKey, kTimelineString)).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "sw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw execution")).Times(1);

  EXPECT_CALL(listener, OnTimer).Times(3);

  event_processor->ProcessEvent(timeline_key_and_string);
  event_processor->ProcessEvent(gpu_job_event_2);

  testing::Mock::VerifyAndClearExpectations(&listener);

  TimerInfo command_buffer_timer_3;
  TimerInfo debug_marker_timer;
  EXPECT_CALL(listener, OnTimer)
      .Times(2)
      .WillOnce(SaveArg<0>(&command_buffer_timer_3))
      .WillOnce(SaveArg<0>(&debug_marker_timer));

  uint64_t actual_command_buffer_key;
  EXPECT_CALL(listener, OnKeyAndString(_, "command buffer"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_command_buffer_key));

  EXPECT_CALL(listener, OnKeyAndString(_, "timeline")).Times(0);
  uint64_t actual_marker_key = 0;
  EXPECT_CALL(listener, OnKeyAndString(_, "marker"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_marker_key));

  event_processor->ProcessEvent(marker_string_event);
  event_processor->ProcessEvent(queue_submission_event_2);
  testing::Mock::VerifyAndClearExpectations(&listener);

  ExpectCommandBufferTimerEq(command_buffer_timer_3, *gpu_job_2, 70, 79, kTimelineKey,
                             actual_command_buffer_key);

  // We expect the begin timestamp to be approximated by the first known timestamp. Also as we don't
  // know the thread id of the begin submission, the timer should state -1 as thread id.
  ExpectDebugMarkerTimerEq(debug_marker_timer, 50, 78, -1, gpu_job_2->pid(), 1, kTimelineKey,
                           actual_marker_key);
}

TEST(CaptureEventProcessor, CanHandleGpuDebugMarkersWithNoBeginJobRecorded) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent timeline_string = CreateInternedStringEvent(kTimelineKey, kTimelineString);

  ClientCaptureEvent gpu_job_event_2;
  GpuJob* gpu_job_2 = CreateGpuJob(&gpu_job_event_2, kTimelineKey, 50, 60, 70, 80);

  ClientCaptureEvent marker_string_event = CreateInternedStringEvent(42, "marker");

  ClientCaptureEvent queue_submission_event_1;
  GpuQueueSubmission* submission_1 = queue_submission_event_1.mutable_gpu_queue_submission();
  GpuQueueSubmissionMetaInfo* meta_info_1 = CreateGpuQueueSubmissionMetaInfo(submission_1, 9, 11);
  GpuSubmitInfo* submit_info_1 = submission_1->add_submit_infos();
  AddGpuCommandBufferToGpuSubmitInfo(submit_info_1, 115, 119);
  AddGpuCommandBufferToGpuSubmitInfo(submit_info_1, 120, 124);
  submission_1->set_num_begin_markers(1);

  ClientCaptureEvent queue_submission_event_2;
  GpuQueueSubmission* submission_2 = queue_submission_event_2.mutable_gpu_queue_submission();
  CreateGpuQueueSubmissionMetaInfo(submission_2, 49, 51);
  GpuSubmitInfo* submit_info_2 = submission_2->add_submit_infos();
  AddGpuCommandBufferToGpuSubmitInfo(submit_info_2, 145, 154);
  AddGpuDebugMarkerToGpuQueueSubmission(submission_2, meta_info_1, 42, 116, 153);

  EXPECT_CALL(listener, OnKeyAndString(kTimelineKey, kTimelineString)).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "sw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw queue")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(_, "hw execution")).Times(1);

  EXPECT_CALL(listener, OnTimer).Times(3);

  event_processor->ProcessEvent(timeline_string);
  event_processor->ProcessEvent(gpu_job_event_2);

  testing::Mock::VerifyAndClearExpectations(&listener);

  event_processor->ProcessEvent(queue_submission_event_1);

  uint64_t actual_command_buffer_key;
  EXPECT_CALL(listener, OnKeyAndString(_, "command buffer"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_command_buffer_key));

  TimerInfo command_buffer_timer_3;
  TimerInfo debug_marker_timer;
  EXPECT_CALL(listener, OnTimer)
      .Times(2)
      .WillOnce(SaveArg<0>(&command_buffer_timer_3))
      .WillOnce(SaveArg<0>(&debug_marker_timer));

  EXPECT_CALL(listener, OnKeyAndString(_, "timeline")).Times(0);
  uint64_t actual_marker_key = 0;
  EXPECT_CALL(listener, OnKeyAndString(_, "marker"))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_marker_key));

  event_processor->ProcessEvent(marker_string_event);
  event_processor->ProcessEvent(queue_submission_event_2);
  testing::Mock::VerifyAndClearExpectations(&listener);

  ExpectCommandBufferTimerEq(command_buffer_timer_3, *gpu_job_2, 70, 79, kTimelineKey,
                             actual_command_buffer_key);

  // We expect the begin timestamp to be approximated by the first known timestamp.
  ExpectDebugMarkerTimerEq(debug_marker_timer, 50, 78, gpu_job_2->tid(), gpu_job_2->pid(), 1,
                           kTimelineKey, actual_marker_key);
}

TEST(CaptureEventProcessor, CanHandleThreadStateSlices) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  ClientCaptureEvent running_event;
  ThreadStateSlice* running_thread_state_slice = running_event.mutable_thread_state_slice();
  running_thread_state_slice->set_duration_ns(100);
  running_thread_state_slice->set_end_timestamp_ns(200);
  running_thread_state_slice->set_pid(14);
  running_thread_state_slice->set_tid(24);
  running_thread_state_slice->set_thread_state(ThreadStateSlice::kRunning);

  ClientCaptureEvent runnable_event;
  ThreadStateSlice* runnable_thread_state_slice = runnable_event.mutable_thread_state_slice();
  runnable_thread_state_slice->set_duration_ns(100);
  runnable_thread_state_slice->set_end_timestamp_ns(200);
  runnable_thread_state_slice->set_pid(14);
  runnable_thread_state_slice->set_tid(24);
  runnable_thread_state_slice->set_thread_state(ThreadStateSlice::kRunnable);

  ClientCaptureEvent dead_event;
  ThreadStateSlice* dead_thread_state_slice = dead_event.mutable_thread_state_slice();
  dead_thread_state_slice->set_duration_ns(100);
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

  event_processor->ProcessEvent(running_event);
  event_processor->ProcessEvent(runnable_event);
  event_processor->ProcessEvent(dead_event);

  EXPECT_EQ(
      actual_running_thread_state_slice_info.begin_timestamp_ns(),
      running_thread_state_slice->end_timestamp_ns() - running_thread_state_slice->duration_ns());
  EXPECT_EQ(actual_running_thread_state_slice_info.end_timestamp_ns(),
            running_thread_state_slice->end_timestamp_ns());
  EXPECT_EQ(actual_running_thread_state_slice_info.tid(), running_thread_state_slice->tid());
  EXPECT_EQ(actual_running_thread_state_slice_info.thread_state(), ThreadStateSliceInfo::kRunning);

  EXPECT_EQ(
      actual_runnable_thread_state_slice_info.begin_timestamp_ns(),
      runnable_thread_state_slice->end_timestamp_ns() - runnable_thread_state_slice->duration_ns());
  EXPECT_EQ(actual_runnable_thread_state_slice_info.end_timestamp_ns(),
            runnable_thread_state_slice->end_timestamp_ns());
  EXPECT_EQ(actual_runnable_thread_state_slice_info.tid(), runnable_thread_state_slice->tid());
  EXPECT_EQ(actual_runnable_thread_state_slice_info.thread_state(),
            ThreadStateSliceInfo::kRunnable);

  EXPECT_EQ(actual_dead_thread_state_slice_info.begin_timestamp_ns(),
            dead_thread_state_slice->end_timestamp_ns() - dead_thread_state_slice->duration_ns());
  EXPECT_EQ(actual_dead_thread_state_slice_info.end_timestamp_ns(),
            dead_thread_state_slice->end_timestamp_ns());
  EXPECT_EQ(actual_dead_thread_state_slice_info.tid(), dead_thread_state_slice->tid());
  EXPECT_EQ(actual_dead_thread_state_slice_info.thread_state(), ThreadStateSliceInfo::kDead);
}

TEST(CaptureEventProcessor, CanHandleMultipleEvents) {
  MockCaptureListener listener;
  auto event_processor = CaptureEventProcessor::CreateForCaptureListener(&listener, {});

  std::vector<ClientCaptureEvent> events;
  ClientCaptureEvent event_1;
  ThreadName* thread_name = event_1.mutable_thread_name();
  thread_name->set_pid(42);
  thread_name->set_tid(24);
  thread_name->set_name("Thread");
  thread_name->set_timestamp_ns(100);
  events.push_back(event_1);

  constexpr uint64_t kFunctionKey = 11;
  constexpr const char* kFunctionName = "Function";
  constexpr uint64_t kModuleKey = 12;
  constexpr const char* kModuleName = "module";
  events.push_back(CreateInternedStringEvent(kFunctionKey, kFunctionName));
  events.push_back(CreateInternedStringEvent(kModuleKey, kModuleName));

  EXPECT_CALL(listener, OnKeyAndString(kFunctionKey, kFunctionName)).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(kModuleKey, kModuleName)).Times(1);
  EXPECT_CALL(listener, OnThreadName(thread_name->tid(), thread_name->name())).Times(1);

  ClientCaptureEvent event_2;
  AddressInfo* address_info = event_2.mutable_address_info();
  address_info->set_absolute_address(42);
  address_info->set_function_name_key(kFunctionKey);
  address_info->set_offset_in_function(14);
  address_info->set_module_name_key(kModuleKey);
  events.push_back(event_2);

  LinuxAddressInfo actual_address_info;
  EXPECT_CALL(listener, OnAddressInfo).Times(1).WillOnce(SaveArg<0>(&actual_address_info));

  event_processor->ProcessEvents(events);

  EXPECT_EQ(actual_address_info.absolute_address(), address_info->absolute_address());
  EXPECT_EQ(actual_address_info.function_name(), kFunctionName);
  EXPECT_EQ(actual_address_info.offset_in_function(), address_info->offset_in_function());
  EXPECT_EQ(actual_address_info.module_path(), kModuleName);
}

}  // namespace orbit_capture_client
