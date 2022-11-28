// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/hash/hash.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "CaptureClient/CaptureEventProcessor.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CgroupAndProcessMemoryInfo.h"
#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/PageFaultsInfo.h"
#include "ClientData/SystemMemoryInfo.h"
#include "ClientData/ThreadStateSliceInfo.h"
#include "ClientData/TracepointEventInfo.h"
#include "ClientData/TracepointInfo.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "MockCaptureListener.h"

namespace orbit_capture_client {

using orbit_client_data::CallstackEvent;
using orbit_client_data::CallstackInfo;
using orbit_client_data::CgroupAndProcessMemoryInfo;
using orbit_client_data::LinuxAddressInfo;
using orbit_client_data::PageFaultsInfo;
using orbit_client_data::SystemMemoryInfo;
using orbit_client_data::ThreadStateSliceInfo;
using orbit_client_data::TracepointEventInfo;

using orbit_client_protos::TimerInfo;

using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CallstackSample;
using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::ClientCaptureEvent;
using orbit_grpc_protos::ClockResolutionEvent;
using orbit_grpc_protos::Color;
using orbit_grpc_protos::ErrorEnablingOrbitApiEvent;
using orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent;
using orbit_grpc_protos::ErrorsWithPerfEventOpenEvent;
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
using orbit_grpc_protos::LostPerfRecordsEvent;
using orbit_grpc_protos::MemoryUsageEvent;
using orbit_grpc_protos::OutOfOrderEventsDiscardedEvent;
using orbit_grpc_protos::PresentEvent;
using orbit_grpc_protos::ProcessMemoryUsage;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadStateSlice;
using orbit_grpc_protos::TracepointEvent;
using orbit_grpc_protos::WarningEvent;
using orbit_grpc_protos::WarningInstrumentingWithUserSpaceInstrumentationEvent;

using ::testing::_;
using ::testing::AllOf;
using ::testing::DoAll;
using ::testing::Field;
using ::testing::Return;
using ::testing::SaveArg;

TEST(CaptureEventProcessor, CanHandleSchedulingSlices) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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

TEST(CaptureEventProcessor, CanHandlePresentEvent) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  PresentEvent* present_event = event.mutable_present_event();
  present_event->set_pid(42);
  present_event->set_tid(24);
  present_event->set_begin_timestamp_ns(100);
  present_event->set_duration_ns(97);
  present_event->set_source(PresentEvent::kD3d9);

  PresentEvent actual_present_event;
  EXPECT_CALL(listener, OnPresentEvent).Times(1).WillOnce(SaveArg<0>(&actual_present_event));
  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_present_event.pid(), present_event->pid());
  EXPECT_EQ(actual_present_event.tid(), present_event->tid());
  EXPECT_EQ(actual_present_event.begin_timestamp_ns(), present_event->begin_timestamp_ns());
  EXPECT_EQ(actual_present_event.duration_ns(), present_event->duration_ns());
  EXPECT_EQ(actual_present_event.source(), present_event->source());
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
  EXPECT_EQ(actual_callstack_event.timestamp_ns(), expected_callstack_sample->timestamp_ns());
  EXPECT_EQ(actual_callstack_event.thread_id(), expected_callstack_sample->tid());
  EXPECT_EQ(actual_callstack_event.callstack_id(), actual_callstack_id);
  ASSERT_EQ(actual_callstack.frames().size(), expected_callstack->pcs_size());
  for (size_t i = 0; i < actual_callstack.frames().size(); ++i) {
    EXPECT_EQ(actual_callstack.frames()[i], expected_callstack->pcs(i));
  }

  EXPECT_EQ(orbit_client_data::GrpcCallstackTypeToCallstackType(expected_callstack->type()),
            actual_callstack.type());
}

static void CanHandleOneCallstackSampleOfType(Callstack::CallstackType type) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent interned_callstack_event;
  InternedCallstack* interned_callstack =
      AddAndInitializeInternedCallstack(interned_callstack_event);
  interned_callstack->mutable_intern()->set_type(type);

  event_processor->ProcessEvent(interned_callstack_event);

  ClientCaptureEvent event;
  CallstackSample* callstack_sample = AddAndInitializeCallstackSample(event);
  callstack_sample->set_timestamp_ns(100);

  uint64_t actual_callstack_id = 0;
  std::optional<CallstackInfo> actual_callstack;
  EXPECT_CALL(listener, OnUniqueCallstack)
      .Times(1)
      .WillOnce([&](uint64_t id, CallstackInfo callstack) {
        actual_callstack_id = id;
        actual_callstack = std::move(callstack);
      });
  std::optional<CallstackEvent> actual_callstack_event;
  EXPECT_CALL(listener, OnCallstackEvent).Times(1).WillOnce(SaveArg<0>(&actual_callstack_event));

  event_processor->ProcessEvent(event);
  ASSERT_TRUE(actual_callstack.has_value());
  ExpectCallstackSamplesEqual(*actual_callstack_event, actual_callstack_id, *actual_callstack,
                              callstack_sample, &interned_callstack->intern());
}

TEST(CaptureEventProcessor, CanHandleOneCallstackSample) {
  CanHandleOneCallstackSampleOfType(Callstack::kComplete);
}

TEST(CaptureEventProcessor, CanHandleOneNonCompleteCallstackSample) {
  CanHandleOneCallstackSampleOfType(Callstack::kDwarfUnwindingError);
  CanHandleOneCallstackSampleOfType(Callstack::kFramePointerUnwindingError);
  CanHandleOneCallstackSampleOfType(Callstack::kInUprobes);
  CanHandleOneCallstackSampleOfType(Callstack::kInUserSpaceInstrumentation);
  CanHandleOneCallstackSampleOfType(Callstack::kCallstackPatchingFailed);
  CanHandleOneCallstackSampleOfType(Callstack::kStackTopDwarfUnwindingError);
  CanHandleOneCallstackSampleOfType(Callstack::kStackTopForDwarfUnwindingTooSmall);
}

TEST(CaptureEventProcessor, WillOnlyHandleUniqueCallstacksOnce) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});
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
  std::optional<CallstackInfo> actual_callstack;
  EXPECT_CALL(listener, OnUniqueCallstack)
      .Times(1)
      .WillOnce([&](uint64_t id, CallstackInfo callstack) {
        actual_callstack_id = id;
        actual_callstack = std::move(callstack);
      });
  std::optional<CallstackEvent> actual_call_stack_event_1;
  std::optional<CallstackEvent> actual_call_stack_event_2;
  EXPECT_CALL(listener, OnCallstackEvent)
      .Times(2)
      .WillOnce(SaveArg<0>(&actual_call_stack_event_1))
      .WillOnce(SaveArg<0>(&actual_call_stack_event_2));

  event_processor->ProcessEvent(event_interned_callstack);
  event_processor->ProcessEvent(event_1);
  event_processor->ProcessEvent(event_2);

  ASSERT_TRUE(actual_callstack.has_value());
  ExpectCallstackSamplesEqual(*actual_call_stack_event_1, actual_callstack_id, *actual_callstack,
                              callstack_sample_1, &interned_callstack->intern());
  ExpectCallstackSamplesEqual(*actual_call_stack_event_2, actual_callstack_id, *actual_callstack,
                              callstack_sample_2, &interned_callstack->intern());
}

TEST(CaptureEventProcessor, CanHandleInternedCallstackSamples) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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
  std::optional<CallstackInfo> actual_callstack;
  EXPECT_CALL(listener, OnUniqueCallstack)
      .Times(1)
      .WillOnce([&](uint64_t id, CallstackInfo callstack) {
        actual_callstack_id = id;
        actual_callstack = std::move(callstack);
      });
  std::optional<CallstackEvent> actual_call_stack_event;
  EXPECT_CALL(listener, OnCallstackEvent).Times(1).WillOnce(SaveArg<0>(&actual_call_stack_event));

  event_processor->ProcessEvent(interned_callstack_event);
  event_processor->ProcessEvent(callstack_event);

  ASSERT_TRUE(actual_callstack.has_value());
  ExpectCallstackSamplesEqual(*actual_call_stack_event, actual_callstack_id, *actual_callstack,
                              callstack_sample, callstack_intern);
}

TEST(CaptureEventProcessor, CanHandleFunctionCalls) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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

TEST(CaptureEventProcessor, CanHandleThreadNames) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  constexpr uint64_t kDemangledFunctionNameKey = 1;
  ClientCaptureEvent interned_demangled_function_name_event =
      CreateInternedStringEvent(kDemangledFunctionNameKey, "already_demangled");

  constexpr uint64_t kMangledFunctionNameKey = 2;
  ClientCaptureEvent interned_mangled_function_name_event =
      CreateInternedStringEvent(kMangledFunctionNameKey, "_Z1hic");

  constexpr uint64_t kModuleNameKey = 3;
  ClientCaptureEvent interned_map_name_event = CreateInternedStringEvent(kModuleNameKey, "module");

  ClientCaptureEvent address_info_with_demangled_name_event;
  AddressInfo* address_info_with_demangled_name =
      address_info_with_demangled_name_event.mutable_address_info();
  address_info_with_demangled_name->set_absolute_address(42);
  address_info_with_demangled_name->set_function_name_key(kDemangledFunctionNameKey);
  address_info_with_demangled_name->set_offset_in_function(14);
  address_info_with_demangled_name->set_module_name_key(kModuleNameKey);

  ClientCaptureEvent address_info_with_mangled_name_event;
  AddressInfo* address_info_with_mangled_name =
      address_info_with_mangled_name_event.mutable_address_info();
  address_info_with_mangled_name->set_absolute_address(43);
  address_info_with_mangled_name->set_function_name_key(kMangledFunctionNameKey);
  address_info_with_mangled_name->set_offset_in_function(15);
  address_info_with_mangled_name->set_module_name_key(kModuleNameKey);

  EXPECT_CALL(listener, OnKeyAndString(kModuleNameKey, "module")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(kDemangledFunctionNameKey, "already_demangled")).Times(1);
  EXPECT_CALL(listener, OnKeyAndString(kMangledFunctionNameKey, "_Z1hic")).Times(1);

  std::optional<LinuxAddressInfo> actual_address_info1;
  std::optional<LinuxAddressInfo> actual_address_info2;
  EXPECT_CALL(listener, OnAddressInfo)
      .Times(2)
      .WillOnce(SaveArg<0>(&actual_address_info1))
      .WillOnce(SaveArg<0>(&actual_address_info2));

  event_processor->ProcessEvent(interned_demangled_function_name_event);
  event_processor->ProcessEvent(interned_mangled_function_name_event);
  event_processor->ProcessEvent(interned_map_name_event);
  event_processor->ProcessEvent(address_info_with_demangled_name_event);
  event_processor->ProcessEvent(address_info_with_mangled_name_event);

  ASSERT_TRUE(actual_address_info1.has_value());
  EXPECT_EQ(actual_address_info1->absolute_address(),
            address_info_with_demangled_name->absolute_address());
  EXPECT_EQ(actual_address_info1->function_name(), "already_demangled");
  EXPECT_EQ(actual_address_info1->offset_in_function(),
            address_info_with_demangled_name->offset_in_function());
  EXPECT_EQ(actual_address_info1->module_path(), "module");

  ASSERT_TRUE(actual_address_info2.has_value());
  EXPECT_EQ(actual_address_info2->absolute_address(),
            address_info_with_mangled_name->absolute_address());
  EXPECT_EQ(actual_address_info2->function_name(), "h(int, char)");
  EXPECT_EQ(actual_address_info2->offset_in_function(),
            address_info_with_mangled_name->offset_in_function());
  EXPECT_EQ(actual_address_info2->module_path(), "module");
}

TEST(CaptureEventProcessor, CanHandleInternedTracepointEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent interned_tracepoint_event;
  InternedTracepointInfo* interned_tracepoint =
      interned_tracepoint_event.mutable_interned_tracepoint_info();
  interned_tracepoint->set_key(2);
  orbit_grpc_protos::TracepointInfo* tracepoint_intern = interned_tracepoint->mutable_intern();
  tracepoint_intern->set_name("name");
  tracepoint_intern->set_category("category");

  ClientCaptureEvent tracepoint_event;
  TracepointEvent* tracepoint = tracepoint_event.mutable_tracepoint_event();
  tracepoint->set_pid(1);
  tracepoint->set_tid(3);
  tracepoint->set_timestamp_ns(100);
  tracepoint->set_cpu(2);
  tracepoint->set_tracepoint_info_key(interned_tracepoint->key());

  uint64_t actual_key{};
  std::optional<orbit_client_data::TracepointInfo> actual_tracepoint_info;
  EXPECT_CALL(listener, OnUniqueTracepointInfo)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_key), SaveArg<1>(&actual_tracepoint_info)));
  std::optional<TracepointEventInfo> actual_tracepoint_event;
  EXPECT_CALL(listener, OnTracepointEvent).Times(1).WillOnce(SaveArg<0>(&actual_tracepoint_event));

  event_processor->ProcessEvent(interned_tracepoint_event);
  event_processor->ProcessEvent(tracepoint_event);

  ASSERT_TRUE(actual_tracepoint_info.has_value());
  EXPECT_EQ(actual_tracepoint_info->category(), tracepoint_intern->category());
  EXPECT_EQ(actual_tracepoint_info->name(), tracepoint_intern->name());

  ASSERT_TRUE(actual_tracepoint_event.has_value());
  EXPECT_EQ(actual_key, actual_tracepoint_event->tracepoint_id());
  EXPECT_EQ(actual_tracepoint_event->tracepoint_id(), tracepoint->tracepoint_info_key());
  EXPECT_EQ(actual_tracepoint_event->pid(), tracepoint->pid());
  EXPECT_EQ(actual_tracepoint_event->tid(), tracepoint->tid());
  EXPECT_EQ(actual_tracepoint_event->timestamp_ns(), tracepoint->timestamp_ns());
  EXPECT_EQ(actual_tracepoint_event->cpu(), tracepoint->cpu());
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
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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

TEST(CaptureEventProcessor, CanHandleMemoryUsageEvent) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  MemoryUsageEvent* memory_usage_event = event.mutable_memory_usage_event();
  SystemMemoryUsage* system_memory_usage = memory_usage_event->mutable_system_memory_usage();
  system_memory_usage->set_timestamp_ns(105);
  system_memory_usage->set_total_kb(10);
  system_memory_usage->set_free_kb(20);
  system_memory_usage->set_available_kb(30);
  system_memory_usage->set_buffers_kb(40);
  system_memory_usage->set_cached_kb(50);
  system_memory_usage->set_pgmajfault(60);
  system_memory_usage->set_pgfault(70);
  CGroupMemoryUsage* cgroup_memory_usage = memory_usage_event->mutable_cgroup_memory_usage();
  cgroup_memory_usage->set_timestamp_ns(110);
  cgroup_memory_usage->set_cgroup_name("memory_cgroup_name");
  cgroup_memory_usage->set_limit_bytes(10);
  cgroup_memory_usage->set_rss_bytes(20);
  cgroup_memory_usage->set_mapped_file_bytes(30);
  cgroup_memory_usage->set_pgmajfault(40);
  cgroup_memory_usage->set_pgfault(50);
  ProcessMemoryUsage* process_memory_usage = memory_usage_event->mutable_process_memory_usage();
  process_memory_usage->set_timestamp_ns(115);
  process_memory_usage->set_pid(1234);
  process_memory_usage->set_rss_anon_kb(10);
  process_memory_usage->set_majflt(20);
  process_memory_usage->set_minflt(30);
  // We take the arithmetic mean of the above events' timestamps as the synchronized timestamp in
  // `MemoryUsageEvent`.
  memory_usage_event->set_timestamp_ns(110);

  uint64_t actual_cgroup_name_key;
  EXPECT_CALL(listener, OnKeyAndString(_, cgroup_memory_usage->cgroup_name()))
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_cgroup_name_key));

  SystemMemoryInfo system_memory_info{};
  EXPECT_CALL(listener, OnSystemMemoryInfo).Times(1).WillOnce(SaveArg<0>(&system_memory_info));

  CgroupAndProcessMemoryInfo cgroup_and_process_memory_info{};
  EXPECT_CALL(listener, OnCgroupAndProcessMemoryInfo)
      .Times(1)
      .WillOnce(SaveArg<0>(&cgroup_and_process_memory_info));

  PageFaultsInfo page_faults_info{};
  EXPECT_CALL(listener, OnPageFaultsInfo).Times(1).WillOnce(SaveArg<0>(&page_faults_info));

  event_processor->ProcessEvent(event);

  EXPECT_THAT(system_memory_info,
              AllOf(Field(&SystemMemoryInfo::timestamp_ns, memory_usage_event->timestamp_ns()),
                    Field(&SystemMemoryInfo::total_kb, system_memory_usage->total_kb()),
                    Field(&SystemMemoryInfo::free_kb, system_memory_usage->free_kb()),
                    Field(&SystemMemoryInfo::available_kb, system_memory_usage->available_kb()),
                    Field(&SystemMemoryInfo::buffers_kb, system_memory_usage->buffers_kb()),
                    Field(&SystemMemoryInfo::cached_kb, system_memory_usage->cached_kb())));

  EXPECT_THAT(
      cgroup_and_process_memory_info,
      AllOf(Field(&CgroupAndProcessMemoryInfo::timestamp_ns, memory_usage_event->timestamp_ns()),
            Field(&CgroupAndProcessMemoryInfo::cgroup_name_hash, actual_cgroup_name_key),
            Field(&CgroupAndProcessMemoryInfo::cgroup_limit_bytes,
                  cgroup_memory_usage->limit_bytes()),
            Field(&CgroupAndProcessMemoryInfo::cgroup_rss_bytes, cgroup_memory_usage->rss_bytes()),
            Field(&CgroupAndProcessMemoryInfo::cgroup_mapped_file_bytes,
                  cgroup_memory_usage->mapped_file_bytes()),
            Field(&CgroupAndProcessMemoryInfo::process_rss_anon_kb,
                  process_memory_usage->rss_anon_kb())));

  EXPECT_THAT(
      page_faults_info,
      AllOf(Field(&PageFaultsInfo::timestamp_ns, memory_usage_event->timestamp_ns()),
            Field(&PageFaultsInfo::system_page_faults, system_memory_usage->pgfault()),
            Field(&PageFaultsInfo::system_major_page_faults, system_memory_usage->pgmajfault()),
            Field(&PageFaultsInfo::cgroup_name_hash, actual_cgroup_name_key),
            Field(&PageFaultsInfo::cgroup_page_faults, cgroup_memory_usage->pgfault()),
            Field(&PageFaultsInfo::cgroup_major_page_faults, cgroup_memory_usage->pgmajfault()),
            Field(&PageFaultsInfo::process_minor_page_faults, process_memory_usage->minflt()),
            Field(&PageFaultsInfo::process_major_page_faults, process_memory_usage->majflt())));
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
                              uint32_t thread_id, uint32_t process_id, uint32_t depth,
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
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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

TEST(CaptureEventProcessor, CanHandleThreadStateSlicesWithoutCallstacks) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent running_event;
  ThreadStateSlice* running_thread_state_slice = running_event.mutable_thread_state_slice();
  running_thread_state_slice->set_duration_ns(100);
  running_thread_state_slice->set_end_timestamp_ns(200);
  running_thread_state_slice->set_pid(14);
  running_thread_state_slice->set_tid(24);
  running_thread_state_slice->set_thread_state(ThreadStateSlice::kRunning);
  running_thread_state_slice->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kNoCallstack);
  running_thread_state_slice->set_switch_out_or_wakeup_callstack_id(0);

  ClientCaptureEvent runnable_event;
  ThreadStateSlice* runnable_thread_state_slice = runnable_event.mutable_thread_state_slice();
  runnable_thread_state_slice->set_duration_ns(100);
  runnable_thread_state_slice->set_end_timestamp_ns(200);
  runnable_thread_state_slice->set_pid(14);
  runnable_thread_state_slice->set_tid(24);
  runnable_thread_state_slice->set_thread_state(ThreadStateSlice::kRunnable);
  runnable_thread_state_slice->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kNoCallstack);
  runnable_thread_state_slice->set_switch_out_or_wakeup_callstack_id(0);

  ClientCaptureEvent dead_event;
  ThreadStateSlice* dead_thread_state_slice = dead_event.mutable_thread_state_slice();
  dead_thread_state_slice->set_duration_ns(100);
  dead_thread_state_slice->set_end_timestamp_ns(200);
  dead_thread_state_slice->set_pid(14);
  dead_thread_state_slice->set_tid(24);
  dead_thread_state_slice->set_thread_state(ThreadStateSlice::kDead);
  dead_thread_state_slice->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kNoCallstack);
  running_thread_state_slice->set_switch_out_or_wakeup_callstack_id(0);

  std::optional<ThreadStateSliceInfo> actual_running_thread_state_slice_info{};
  std::optional<ThreadStateSliceInfo> actual_runnable_thread_state_slice_info{};
  std::optional<ThreadStateSliceInfo> actual_dead_thread_state_slice_info{};
  EXPECT_CALL(listener, OnThreadStateSlice)
      .Times(3)
      .WillOnce(SaveArg<0>(&actual_running_thread_state_slice_info))
      .WillOnce(SaveArg<0>(&actual_runnable_thread_state_slice_info))
      .WillOnce(SaveArg<0>(&actual_dead_thread_state_slice_info));

  event_processor->ProcessEvent(running_event);
  event_processor->ProcessEvent(runnable_event);
  event_processor->ProcessEvent(dead_event);

  ASSERT_TRUE(actual_running_thread_state_slice_info.has_value());
  EXPECT_EQ(
      actual_running_thread_state_slice_info->begin_timestamp_ns(),
      running_thread_state_slice->end_timestamp_ns() - running_thread_state_slice->duration_ns());
  EXPECT_EQ(actual_running_thread_state_slice_info->end_timestamp_ns(),
            running_thread_state_slice->end_timestamp_ns());
  EXPECT_EQ(actual_running_thread_state_slice_info->tid(), running_thread_state_slice->tid());
  EXPECT_EQ(actual_running_thread_state_slice_info->thread_state(), ThreadStateSlice::kRunning);
  EXPECT_EQ(actual_running_thread_state_slice_info->switch_out_or_wakeup_callstack_id(),
            std::nullopt);

  ASSERT_TRUE(actual_runnable_thread_state_slice_info.has_value());
  EXPECT_EQ(
      actual_runnable_thread_state_slice_info->begin_timestamp_ns(),
      runnable_thread_state_slice->end_timestamp_ns() - runnable_thread_state_slice->duration_ns());
  EXPECT_EQ(actual_runnable_thread_state_slice_info->end_timestamp_ns(),
            runnable_thread_state_slice->end_timestamp_ns());
  EXPECT_EQ(actual_runnable_thread_state_slice_info->tid(), runnable_thread_state_slice->tid());
  EXPECT_EQ(actual_runnable_thread_state_slice_info->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(actual_runnable_thread_state_slice_info->switch_out_or_wakeup_callstack_id(),
            std::nullopt);

  ASSERT_TRUE(actual_dead_thread_state_slice_info.has_value());
  EXPECT_EQ(actual_dead_thread_state_slice_info->begin_timestamp_ns(),
            dead_thread_state_slice->end_timestamp_ns() - dead_thread_state_slice->duration_ns());
  EXPECT_EQ(actual_dead_thread_state_slice_info->end_timestamp_ns(),
            dead_thread_state_slice->end_timestamp_ns());
  EXPECT_EQ(actual_dead_thread_state_slice_info->tid(), dead_thread_state_slice->tid());
  EXPECT_EQ(actual_dead_thread_state_slice_info->thread_state(), ThreadStateSlice::kDead);
  EXPECT_EQ(actual_dead_thread_state_slice_info->switch_out_or_wakeup_callstack_id(), std::nullopt);
}

TEST(CaptureEventProcessor, DeathOnThreadStateSlicesWithUnknownCallstack) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  constexpr uint64_t kCallstackId = 24;

  ClientCaptureEvent thread_state_slice_without_known_callstack_id_event;
  ThreadStateSlice* thread_state_slice_without_known_callstack_id =
      thread_state_slice_without_known_callstack_id_event.mutable_thread_state_slice();
  thread_state_slice_without_known_callstack_id->set_duration_ns(100);
  thread_state_slice_without_known_callstack_id->set_end_timestamp_ns(200);
  thread_state_slice_without_known_callstack_id->set_pid(14);
  thread_state_slice_without_known_callstack_id->set_tid(24);
  thread_state_slice_without_known_callstack_id->set_thread_state(ThreadStateSlice::kRunnable);
  thread_state_slice_without_known_callstack_id->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kCallstackSet);
  thread_state_slice_without_known_callstack_id->set_switch_out_or_wakeup_callstack_id(
      kCallstackId);

  ClientCaptureEvent thread_state_slice_waiting_for_callstack_event;
  ThreadStateSlice* thread_state_slice_waiting_for_callstack =
      thread_state_slice_waiting_for_callstack_event.mutable_thread_state_slice();
  thread_state_slice_waiting_for_callstack->set_duration_ns(100);
  thread_state_slice_waiting_for_callstack->set_end_timestamp_ns(200);
  thread_state_slice_waiting_for_callstack->set_pid(14);
  thread_state_slice_waiting_for_callstack->set_tid(24);
  thread_state_slice_waiting_for_callstack->set_thread_state(ThreadStateSlice::kInterruptibleSleep);
  thread_state_slice_waiting_for_callstack->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kWaitingForCallstack);
  thread_state_slice_waiting_for_callstack->set_switch_out_or_wakeup_callstack_id(0);

  EXPECT_CALL(listener, OnUniqueCallstack).Times(0);
  EXPECT_CALL(listener, OnThreadStateSlice).Times(0);

  EXPECT_DEATH(event_processor->ProcessEvent(thread_state_slice_without_known_callstack_id_event),
               "");
  EXPECT_DEATH(event_processor->ProcessEvent(thread_state_slice_waiting_for_callstack_event), "");
}

TEST(CaptureEventProcessor, CanHandleThreadStateSlicesWithCallstacks) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  constexpr uint64_t kCallstackId = 24;
  constexpr uint64_t kFrame1 = 1;
  constexpr uint64_t kFrame2 = 2;
  constexpr uint64_t kFrame3 = 3;

  ClientCaptureEvent interned_callstack_event;
  InternedCallstack* interned_callstack = interned_callstack_event.mutable_interned_callstack();
  interned_callstack->set_key(kCallstackId);
  interned_callstack->mutable_intern()->add_pcs(kFrame1);
  interned_callstack->mutable_intern()->add_pcs(kFrame2);
  interned_callstack->mutable_intern()->add_pcs(kFrame3);
  interned_callstack->mutable_intern()->set_type(orbit_grpc_protos::Callstack::kComplete);

  ClientCaptureEvent runnable_thread_state_slice_event;
  ThreadStateSlice* runnable_thread_state_slice =
      runnable_thread_state_slice_event.mutable_thread_state_slice();
  runnable_thread_state_slice->set_duration_ns(100);
  runnable_thread_state_slice->set_end_timestamp_ns(200);
  runnable_thread_state_slice->set_pid(14);
  runnable_thread_state_slice->set_tid(24);
  runnable_thread_state_slice->set_thread_state(ThreadStateSlice::kRunnable);
  runnable_thread_state_slice->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kCallstackSet);
  runnable_thread_state_slice->set_switch_out_or_wakeup_callstack_id(kCallstackId);

  std::optional<uint64_t> actual_callstack_id;
  std::optional<CallstackInfo> actual_callstack;
  EXPECT_CALL(listener, OnUniqueCallstack)
      .Times(1)
      .WillOnce(DoAll(SaveArg<0>(&actual_callstack_id), SaveArg<1>(&actual_callstack)));

  std::optional<ThreadStateSliceInfo> actual_runnable_thread_state_slice_info{};
  EXPECT_CALL(listener, OnThreadStateSlice)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_runnable_thread_state_slice_info));

  event_processor->ProcessEvent(interned_callstack_event);
  event_processor->ProcessEvent(runnable_thread_state_slice_event);

  ASSERT_TRUE(actual_callstack_id.has_value());
  ASSERT_TRUE(actual_callstack.has_value());
  EXPECT_EQ(actual_callstack_id.value(), kCallstackId);
  EXPECT_EQ(actual_callstack->type(), orbit_client_data::CallstackType::kComplete);
  EXPECT_EQ(actual_callstack->frames().size(), 3);
  EXPECT_EQ(actual_callstack->frames()[0], kFrame1);
  EXPECT_EQ(actual_callstack->frames()[1], kFrame2);
  EXPECT_EQ(actual_callstack->frames()[2], kFrame3);

  ASSERT_TRUE(actual_runnable_thread_state_slice_info.has_value());
  EXPECT_EQ(
      actual_runnable_thread_state_slice_info->begin_timestamp_ns(),
      runnable_thread_state_slice->end_timestamp_ns() - runnable_thread_state_slice->duration_ns());
  EXPECT_EQ(actual_runnable_thread_state_slice_info->end_timestamp_ns(),
            runnable_thread_state_slice->end_timestamp_ns());
  EXPECT_EQ(actual_runnable_thread_state_slice_info->tid(), runnable_thread_state_slice->tid());
  EXPECT_EQ(actual_runnable_thread_state_slice_info->thread_state(), ThreadStateSlice::kRunnable);
  EXPECT_EQ(actual_runnable_thread_state_slice_info->switch_out_or_wakeup_callstack_id(),
            kCallstackId);
}

TEST(CaptureEventProcessor, CanHandleWarningEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  WarningEvent* warning_event = event.mutable_warning_event();
  constexpr uint64_t kTimestampNs = 100;
  warning_event->set_timestamp_ns(kTimestampNs);
  constexpr const char* kMessage = "message";
  warning_event->set_message(kMessage);

  WarningEvent actual_warning_event;
  EXPECT_CALL(listener, OnWarningEvent).Times(1).WillOnce(SaveArg<0>(&actual_warning_event));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_warning_event.timestamp_ns(), kTimestampNs);
  EXPECT_EQ(actual_warning_event.message(), kMessage);
}

TEST(CaptureEventProcessor, CanHandleClockResolutionEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  ClockResolutionEvent* clock_resolution_event = event.mutable_clock_resolution_event();
  constexpr uint64_t kTimestampNs = 100;
  clock_resolution_event->set_timestamp_ns(kTimestampNs);
  constexpr uint64_t kClockResolutionNs = 123;
  clock_resolution_event->set_clock_resolution_ns(kClockResolutionNs);

  ClockResolutionEvent actual_clock_resolution_event;
  EXPECT_CALL(listener, OnClockResolutionEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_clock_resolution_event));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_clock_resolution_event.timestamp_ns(), kTimestampNs);
  EXPECT_EQ(actual_clock_resolution_event.clock_resolution_ns(), kClockResolutionNs);
}

TEST(CaptureEventProcessor, CanHandleErrorsWithPerfEventOpenEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  ErrorsWithPerfEventOpenEvent* errors_with_perf_event_open_event =
      event.mutable_errors_with_perf_event_open_event();
  constexpr uint64_t kTimestampNs = 100;
  errors_with_perf_event_open_event->set_timestamp_ns(kTimestampNs);
  constexpr const char* kFailedToOpen1 = "sampling";
  constexpr const char* kFailedToOpen2 = "uprobes";
  errors_with_perf_event_open_event->add_failed_to_open(kFailedToOpen1);
  errors_with_perf_event_open_event->add_failed_to_open(kFailedToOpen2);

  ErrorsWithPerfEventOpenEvent actual_errors_with_perf_event_open_event;
  EXPECT_CALL(listener, OnErrorsWithPerfEventOpenEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_errors_with_perf_event_open_event));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_errors_with_perf_event_open_event.timestamp_ns(), kTimestampNs);
  EXPECT_THAT(std::vector(actual_errors_with_perf_event_open_event.failed_to_open().begin(),
                          actual_errors_with_perf_event_open_event.failed_to_open().end()),
              testing::ElementsAre(kFailedToOpen1, kFailedToOpen2));
}

TEST(CaptureEventProcessor, CanHandleErrorEnablingOrbitApiEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  ErrorEnablingOrbitApiEvent* error_enabling_orbit_api_event =
      event.mutable_error_enabling_orbit_api_event();
  constexpr uint64_t kTimestampNs = 100;
  error_enabling_orbit_api_event->set_timestamp_ns(kTimestampNs);
  constexpr const char* kMessage = "message";
  error_enabling_orbit_api_event->set_message(kMessage);

  ErrorEnablingOrbitApiEvent actual_error_enabling_orbit_api_event;
  EXPECT_CALL(listener, OnErrorEnablingOrbitApiEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_error_enabling_orbit_api_event));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_error_enabling_orbit_api_event.timestamp_ns(), kTimestampNs);
  EXPECT_EQ(actual_error_enabling_orbit_api_event.message(), kMessage);
}

TEST(CaptureEventProcessor, CanHandleErrorEnablingUserSpaceInstrumentationEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  ErrorEnablingUserSpaceInstrumentationEvent* error_event =
      event.mutable_error_enabling_user_space_instrumentation_event();
  constexpr uint64_t kTimestampNs = 100;
  error_event->set_timestamp_ns(kTimestampNs);
  constexpr const char* kMessage = "message";
  error_event->set_message(kMessage);

  ErrorEnablingUserSpaceInstrumentationEvent actual_error_event;
  EXPECT_CALL(listener, OnErrorEnablingUserSpaceInstrumentationEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_error_event));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_error_event.timestamp_ns(), kTimestampNs);
  EXPECT_EQ(actual_error_event.message(), kMessage);
}

TEST(CaptureEventProcessor, CanHandleWarningInstrumentingWithUserSpaceInstrumentationEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  WarningInstrumentingWithUserSpaceInstrumentationEvent* warning_event =
      event.mutable_warning_instrumenting_with_user_space_instrumentation_event();
  constexpr uint64_t kTimestampNs = 100;
  warning_event->set_timestamp_ns(kTimestampNs);
  constexpr uint64_t kFunctionId = 42;
  constexpr const char* kErrorMessage = "error message";
  orbit_grpc_protos::FunctionThatFailedToBeInstrumented* function =
      warning_event->add_functions_that_failed_to_instrument();
  function->set_function_id(kFunctionId);
  function->set_error_message(kErrorMessage);

  WarningInstrumentingWithUserSpaceInstrumentationEvent actual_warning_event;
  EXPECT_CALL(listener, OnWarningInstrumentingWithUserSpaceInstrumentationEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_warning_event));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_warning_event.timestamp_ns(), kTimestampNs);
  EXPECT_EQ(actual_warning_event.functions_that_failed_to_instrument(0).function_id(), kFunctionId);
  EXPECT_EQ(actual_warning_event.functions_that_failed_to_instrument(0).error_message(),
            kErrorMessage);
}

TEST(CaptureEventProcessor, CanHandleLostPerfRecordsEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  LostPerfRecordsEvent* lost_perf_records_event = event.mutable_lost_perf_records_event();
  constexpr uint64_t kDurationNs = 42;
  lost_perf_records_event->set_duration_ns(kDurationNs);
  constexpr uint64_t kEndTimestampNs = 123;
  lost_perf_records_event->set_end_timestamp_ns(kEndTimestampNs);

  LostPerfRecordsEvent actual_lost_perf_records_event;
  EXPECT_CALL(listener, OnLostPerfRecordsEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_lost_perf_records_event));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_lost_perf_records_event.duration_ns(), kDurationNs);
  EXPECT_EQ(actual_lost_perf_records_event.end_timestamp_ns(), kEndTimestampNs);
}

TEST(CaptureEventProcessor, CanHandleOutOfOrderEventsDiscardedEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

  ClientCaptureEvent event;
  OutOfOrderEventsDiscardedEvent* out_of_order_events_discarded_event =
      event.mutable_out_of_order_events_discarded_event();
  constexpr uint64_t kDurationNs = 42;
  out_of_order_events_discarded_event->set_duration_ns(kDurationNs);
  constexpr uint64_t kEndTimestampNs = 123;
  out_of_order_events_discarded_event->set_end_timestamp_ns(kEndTimestampNs);

  OutOfOrderEventsDiscardedEvent actual_out_of_order_events_discarded_event;
  EXPECT_CALL(listener, OnOutOfOrderEventsDiscardedEvent)
      .Times(1)
      .WillOnce(SaveArg<0>(&actual_out_of_order_events_discarded_event));

  event_processor->ProcessEvent(event);

  EXPECT_EQ(actual_out_of_order_events_discarded_event.duration_ns(), kDurationNs);
  EXPECT_EQ(actual_out_of_order_events_discarded_event.end_timestamp_ns(), kEndTimestampNs);
}

TEST(CaptureEventProcessor, CanHandleMultipleEvents) {
  MockCaptureListener listener;
  auto event_processor =
      CaptureEventProcessor::CreateForCaptureListener(&listener, std::filesystem::path{}, {});

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

  std::optional<LinuxAddressInfo> actual_address_info;
  EXPECT_CALL(listener, OnAddressInfo).Times(1).WillOnce(SaveArg<0>(&actual_address_info));

  for (const auto& event : events) {
    event_processor->ProcessEvent(event);
  }

  ASSERT_TRUE(actual_address_info.has_value());
  EXPECT_EQ(actual_address_info->absolute_address(), address_info->absolute_address());
  EXPECT_EQ(actual_address_info->function_name(), kFunctionName);
  EXPECT_EQ(actual_address_info->offset_in_function(), address_info->offset_in_function());
  EXPECT_EQ(actual_address_info->module_path(), kModuleName);
}

}  // namespace orbit_capture_client
