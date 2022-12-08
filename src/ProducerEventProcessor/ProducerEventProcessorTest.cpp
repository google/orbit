// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "GrpcProtos/Constants.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "ProducerEventProcessor/ClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

using orbit_grpc_protos::AddressInfo;
using orbit_grpc_protos::ApiScopeStart;
using orbit_grpc_protos::ApiScopeStartAsync;
using orbit_grpc_protos::ApiScopeStop;
using orbit_grpc_protos::ApiScopeStopAsync;
using orbit_grpc_protos::ApiStringEvent;
using orbit_grpc_protos::ApiTrackDouble;
using orbit_grpc_protos::ApiTrackFloat;
using orbit_grpc_protos::ApiTrackInt;
using orbit_grpc_protos::ApiTrackInt64;
using orbit_grpc_protos::ApiTrackUint;
using orbit_grpc_protos::ApiTrackUint64;
using orbit_grpc_protos::Callstack;
using orbit_grpc_protos::CallstackSample;
using orbit_grpc_protos::CaptureFinished;
using orbit_grpc_protos::CaptureOptions;
using orbit_grpc_protos::CaptureStarted;
using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::ClientCaptureEvent;
using orbit_grpc_protos::ClockResolutionEvent;
using orbit_grpc_protos::ErrorEnablingOrbitApiEvent;
using orbit_grpc_protos::ErrorEnablingUserSpaceInstrumentationEvent;
using orbit_grpc_protos::ErrorsWithPerfEventOpenEvent;
using orbit_grpc_protos::FullAddressInfo;
using orbit_grpc_protos::FullCallstackSample;
using orbit_grpc_protos::FullGpuJob;
using orbit_grpc_protos::FullTracepointEvent;
using orbit_grpc_protos::FunctionCall;
using orbit_grpc_protos::GpuCommandBuffer;
using orbit_grpc_protos::GpuDebugMarker;
using orbit_grpc_protos::GpuJob;
using orbit_grpc_protos::GpuQueueSubmission;
using orbit_grpc_protos::GpuSubmitInfo;
using orbit_grpc_protos::InstrumentedFunction;
using orbit_grpc_protos::InternedCallstack;
using orbit_grpc_protos::InternedString;
using orbit_grpc_protos::InternedTracepointInfo;
using orbit_grpc_protos::kMemoryInfoProducerId;
using orbit_grpc_protos::LostPerfRecordsEvent;
using orbit_grpc_protos::MemoryUsageEvent;
using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModulesSnapshot;
using orbit_grpc_protos::ModuleUpdateEvent;
using orbit_grpc_protos::OutOfOrderEventsDiscardedEvent;
using orbit_grpc_protos::ProcessMemoryUsage;
using orbit_grpc_protos::ProducerCaptureEvent;
using orbit_grpc_protos::SchedulingSlice;
using orbit_grpc_protos::SystemMemoryUsage;
using orbit_grpc_protos::ThreadName;
using orbit_grpc_protos::ThreadNamesSnapshot;
using orbit_grpc_protos::ThreadStateSlice;
using orbit_grpc_protos::ThreadStateSliceCallstack;
using orbit_grpc_protos::TracepointEvent;
using orbit_grpc_protos::WarningEvent;
using orbit_grpc_protos::WarningInstrumentingWithUserSpaceInstrumentationEvent;

using google::protobuf::util::MessageDifferencer;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Invoke;
using ::testing::SaveArg;

namespace orbit_producer_event_processor {

namespace {

class MockClientCaptureEventCollector : public ClientCaptureEventCollector {
 public:
  MOCK_METHOD(void, AddEvent, (orbit_grpc_protos::ClientCaptureEvent && /*event*/), (override));
  MOCK_METHOD(void, StopAndWait, (), (override));
};

constexpr uint64_t kDefaultProducerId = 31;

constexpr int32_t kPid1 = 5;
constexpr int32_t kPid2 = 17;
constexpr int32_t kTid1 = 7;
constexpr int32_t kTid2 = 111;
constexpr int32_t kCore1 = 11;
constexpr uint64_t kKey1 = 13;
constexpr uint64_t kKey2 = 113;

constexpr uint64_t kDurationNs1 = 971;
constexpr uint64_t kDurationNs2 = 977;

constexpr uint64_t kTimestampNs1 = 7723;
constexpr uint64_t kTimestampNs2 = 7727;

constexpr int32_t kNumBeginMarkers1 = 19;
constexpr int32_t kNumBeginMarkers2 = 23;

constexpr int32_t kDepth1 = 29;
constexpr int32_t kDepth2 = 31;

constexpr uint64_t kFunctionId1 = 37;
constexpr uint64_t kFunctionId2 = 41;

constexpr uint32_t kGpuJobContext1 = 43;
constexpr uint32_t kGpuJobContext2 = 47;

constexpr uint32_t kSeqNo1 = 53;
constexpr uint32_t kSeqNo2 = 59;

constexpr float kAlpha1 = 0.1f;
constexpr float kRed1 = 0.2f;
constexpr float kGreen1 = 0.3f;
constexpr float kBlue1 = 0.4f;

constexpr float kAlpha2 = 2.1f;
constexpr float kRed2 = 2.2f;
constexpr float kGreen2 = 2.3f;
constexpr float kBlue2 = 2.4f;

constexpr uint32_t kColor1 = 0x11223344;

constexpr uint64_t kGroupId1 = 42;

constexpr uint64_t kEncodedName1 = 11;
constexpr uint64_t kEncodedName2 = 22;
constexpr uint64_t kEncodedName3 = 33;
constexpr uint64_t kEncodedName4 = 44;
constexpr uint64_t kEncodedName5 = 55;
constexpr uint64_t kEncodedName6 = 66;
constexpr uint64_t kEncodedName7 = 77;
constexpr uint64_t kEncodedName8 = 88;
constexpr uint64_t kEncodedNameAdditional1 = 99;

constexpr int32_t kInt = 42;
constexpr int64_t kInt64 = 64;
constexpr uint32_t kUint = 42;
constexpr uint64_t kUint64 = 64;
constexpr float kFloat = 1.1f;
constexpr double kDouble = 2.2;

constexpr const char* kExecutablePath = "/path/to/executable";
constexpr const char* kBuildId1 = "build_id_1";
constexpr const char* kBuildId2 = "build_id_2";

}  // namespace

TEST(ProducerEventProcessor, OneSchedulingSliceEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event;
  SchedulingSlice* scheduling_slice = event.mutable_scheduling_slice();
  scheduling_slice->set_pid(kPid1);
  scheduling_slice->set_tid(kTid1);
  scheduling_slice->set_core(kCore1);
  scheduling_slice->set_duration_ns(kDurationNs1);
  scheduling_slice->set_out_timestamp_ns(kTimestampNs1);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kSchedulingSlice);
  const SchedulingSlice& actual_scheduling_slice = client_capture_event.scheduling_slice();

  EXPECT_EQ(actual_scheduling_slice.pid(), kPid1);
  EXPECT_EQ(actual_scheduling_slice.tid(), kTid1);
  EXPECT_EQ(actual_scheduling_slice.core(), kCore1);
  EXPECT_EQ(actual_scheduling_slice.duration_ns(), kDurationNs1);
  EXPECT_EQ(actual_scheduling_slice.out_timestamp_ns(), kTimestampNs1);
}

TEST(ProducerEventProcessor, OneInternedCallstack) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event;
  InternedCallstack* interned_callstack = event.mutable_interned_callstack();
  interned_callstack->set_key(kKey1);
  Callstack* callstack = interned_callstack->mutable_intern();
  callstack->add_pcs(1);
  callstack->add_pcs(2);
  callstack->add_pcs(3);
  callstack->set_type(Callstack::kComplete);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(event));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kInternedCallstack);
  const InternedCallstack& actual_interned_callstack = client_capture_event.interned_callstack();

  // We do not expect resulting id to be the same, but we also do not
  // enforce sequential ids in the test. 0 value is reserved.
  EXPECT_NE(actual_interned_callstack.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(actual_interned_callstack.intern().pcs_size(), 3);
  EXPECT_EQ(actual_interned_callstack.intern().pcs()[0], 1);
  EXPECT_EQ(actual_interned_callstack.intern().pcs()[1], 2);
  EXPECT_EQ(actual_interned_callstack.intern().pcs()[2], 3);
}

TEST(ProducerEventProcessor, TwoInternedCallstacksSameFramesDifferentTypes) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  InternedCallstack* interned_callstack1 = event1.mutable_interned_callstack();
  interned_callstack1->set_key(kKey1);
  Callstack* callstack1 = interned_callstack1->mutable_intern();
  callstack1->add_pcs(1);
  callstack1->add_pcs(2);
  callstack1->add_pcs(3);
  callstack1->set_type(Callstack::kComplete);

  ProducerCaptureEvent event2;
  InternedCallstack* interned_callstack2 = event2.mutable_interned_callstack();
  interned_callstack2->set_key(kKey2);
  Callstack* callstack2 = interned_callstack2->mutable_intern();
  callstack2->add_pcs(1);
  callstack2->add_pcs(2);
  callstack2->add_pcs(3);
  callstack2->set_type(Callstack::kDwarfUnwindingError);

  ClientCaptureEvent client_capture_event1;
  ClientCaptureEvent client_capture_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(2)
      .WillOnce(SaveArg<0>(&client_capture_event1))
      .WillOnce(SaveArg<0>(&client_capture_event2));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(event1));
  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(event2));

  ASSERT_EQ(client_capture_event1.event_case(), ClientCaptureEvent::kInternedCallstack);
  ASSERT_EQ(client_capture_event2.event_case(), ClientCaptureEvent::kInternedCallstack);

  const InternedCallstack& actual_interned_callstack1 = client_capture_event1.interned_callstack();
  const InternedCallstack& actual_interned_callstack2 = client_capture_event2.interned_callstack();

  EXPECT_NE(actual_interned_callstack1.key(), actual_interned_callstack2.key());

  EXPECT_NE(actual_interned_callstack1.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(actual_interned_callstack1.intern().pcs_size(), 3);
  EXPECT_EQ(actual_interned_callstack1.intern().pcs()[0], 1);
  EXPECT_EQ(actual_interned_callstack1.intern().pcs()[1], 2);
  EXPECT_EQ(actual_interned_callstack1.intern().pcs()[2], 3);
  EXPECT_EQ(actual_interned_callstack1.intern().type(), Callstack::kComplete);

  EXPECT_NE(actual_interned_callstack2.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(actual_interned_callstack2.intern().pcs_size(), 3);
  EXPECT_EQ(actual_interned_callstack2.intern().pcs()[0], 1);
  EXPECT_EQ(actual_interned_callstack2.intern().pcs()[1], 2);
  EXPECT_EQ(actual_interned_callstack2.intern().pcs()[2], 3);
  EXPECT_EQ(actual_interned_callstack2.intern().type(), Callstack::kDwarfUnwindingError);
}

TEST(ProducerEventProcessor, TwoInternedCallstacksDifferentProducersSameKey) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  InternedCallstack* interned_callstack1 = event1.mutable_interned_callstack();
  interned_callstack1->set_key(kKey1);
  Callstack* callstack1 = interned_callstack1->mutable_intern();
  callstack1->add_pcs(1);
  callstack1->add_pcs(2);
  callstack1->add_pcs(3);
  callstack1->set_type(Callstack::kComplete);

  ProducerCaptureEvent event2;
  InternedCallstack* interned_callstack2 = event2.mutable_interned_callstack();
  interned_callstack2->set_key(kKey1);
  Callstack* callstack2 = interned_callstack2->mutable_intern();
  callstack2->add_pcs(1);
  callstack2->add_pcs(2);
  callstack2->add_pcs(4);
  callstack2->set_type(Callstack::kComplete);

  ClientCaptureEvent client_capture_event1;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event1));
  producer_event_processor->ProcessEvent(1, std::move(event1));

  ClientCaptureEvent client_capture_event2;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event2));
  producer_event_processor->ProcessEvent(2, std::move(event2));

  ASSERT_EQ(client_capture_event1.event_case(), ClientCaptureEvent::kInternedCallstack);
  ASSERT_EQ(client_capture_event2.event_case(), ClientCaptureEvent::kInternedCallstack);

  const InternedCallstack& actual_interned_callstack1 = client_capture_event1.interned_callstack();
  const InternedCallstack& actual_interned_callstack2 = client_capture_event2.interned_callstack();

  EXPECT_NE(actual_interned_callstack1.key(), actual_interned_callstack2.key());

  EXPECT_NE(actual_interned_callstack1.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(actual_interned_callstack1.intern().pcs_size(), 3);
  EXPECT_EQ(actual_interned_callstack1.intern().pcs()[0], 1);
  EXPECT_EQ(actual_interned_callstack1.intern().pcs()[1], 2);
  EXPECT_EQ(actual_interned_callstack1.intern().pcs()[2], 3);
  EXPECT_EQ(actual_interned_callstack1.intern().type(), Callstack::kComplete);

  EXPECT_NE(actual_interned_callstack2.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(actual_interned_callstack2.intern().pcs_size(), 3);
  EXPECT_EQ(actual_interned_callstack2.intern().pcs()[0], 1);
  EXPECT_EQ(actual_interned_callstack2.intern().pcs()[1], 2);
  EXPECT_EQ(actual_interned_callstack2.intern().pcs()[2], 4);
  EXPECT_EQ(actual_interned_callstack2.intern().type(), Callstack::kComplete);
}

TEST(ProducerEventProcessor, TwoInternedCallstacksDifferentProducersSameIntern) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  constexpr uint64_t kProducer1CallstackKey = kKey1;
  constexpr uint64_t kProducer2CallstackKey = kKey2;

  ProducerCaptureEvent event1;
  InternedCallstack* interned_callstack1 = event1.mutable_interned_callstack();
  interned_callstack1->set_key(kProducer1CallstackKey);
  Callstack* callstack1 = interned_callstack1->mutable_intern();
  callstack1->add_pcs(1);
  callstack1->add_pcs(2);
  callstack1->add_pcs(3);
  callstack1->set_type(Callstack::kComplete);

  ProducerCaptureEvent event2;
  InternedCallstack* interned_callstack2 = event2.mutable_interned_callstack();
  interned_callstack2->set_key(kProducer2CallstackKey);
  interned_callstack2->mutable_intern()->CopyFrom(*callstack1);

  ClientCaptureEvent client_capture_event;
  // We expect only one call here because we have same callstacks.
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));
  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(2, std::move(event2));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kInternedCallstack);
  const InternedCallstack& actual_interned_callstack = client_capture_event.interned_callstack();

  EXPECT_NE(actual_interned_callstack.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(actual_interned_callstack.intern().pcs_size(), 3);
  EXPECT_EQ(actual_interned_callstack.intern().pcs()[0], 1);
  EXPECT_EQ(actual_interned_callstack.intern().pcs()[1], 2);
  EXPECT_EQ(actual_interned_callstack.intern().pcs()[2], 3);
  EXPECT_EQ(actual_interned_callstack.intern().type(), Callstack::kComplete);

  // Now check that the both producer's callstack are still tracked by
  // ProducerEventProcessor
  testing::Mock::VerifyAndClearExpectations(&collector);

  ProducerCaptureEvent callstack_sample_event1;
  CallstackSample* callstack_sample1 = callstack_sample_event1.mutable_callstack_sample();
  callstack_sample1->set_pid(kPid1);
  callstack_sample1->set_tid(kTid1);
  callstack_sample1->set_timestamp_ns(kTimestampNs1);
  callstack_sample1->set_callstack_id(kProducer1CallstackKey);

  ProducerCaptureEvent callstack_sample_event2;
  CallstackSample* callstack_sample2 = callstack_sample_event2.mutable_callstack_sample();
  callstack_sample2->set_pid(kPid2);
  callstack_sample2->set_tid(kTid2);
  callstack_sample2->set_timestamp_ns(kTimestampNs2);
  callstack_sample2->set_callstack_id(kProducer2CallstackKey);

  ClientCaptureEvent sample_capture_event1;
  ClientCaptureEvent sample_capture_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(2)
      .WillOnce(SaveArg<0>(&sample_capture_event1))
      .WillOnce(SaveArg<0>(&sample_capture_event2));
  producer_event_processor->ProcessEvent(1, std::move(callstack_sample_event1));
  producer_event_processor->ProcessEvent(2, std::move(callstack_sample_event2));

  ASSERT_EQ(sample_capture_event1.event_case(), ClientCaptureEvent::kCallstackSample);
  ASSERT_EQ(sample_capture_event2.event_case(), ClientCaptureEvent::kCallstackSample);

  const CallstackSample& sample1 = sample_capture_event1.callstack_sample();
  const CallstackSample& sample2 = sample_capture_event2.callstack_sample();

  EXPECT_EQ(sample1.pid(), kPid1);
  EXPECT_EQ(sample1.tid(), kTid1);
  EXPECT_EQ(sample1.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(sample1.callstack_id(), actual_interned_callstack.key());

  EXPECT_EQ(sample2.pid(), kPid2);
  EXPECT_EQ(sample2.tid(), kTid2);
  EXPECT_EQ(sample2.timestamp_ns(), kTimestampNs2);
  EXPECT_EQ(sample2.callstack_id(), actual_interned_callstack.key());
}

static ProducerCaptureEvent CreateInternedStringEvent(uint64_t key, std::string str) {
  ProducerCaptureEvent interned_string_event;
  InternedString* interned_string = interned_string_event.mutable_interned_string();
  interned_string->set_key(key);
  interned_string->set_intern(std::move(str));
  return interned_string_event;
}

TEST(ProducerEventProcessor, OneInternedString) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event = CreateInternedStringEvent(kKey1, "string");

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(event));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kInternedString);
  const InternedString& actual_interned_string = client_capture_event.interned_string();

  // We do not expect resulting id to be the same, but we also do not
  // enforce sequential ids in the test. 0 value is reserved.
  EXPECT_NE(actual_interned_string.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(actual_interned_string.intern(), "string");
}

TEST(ProducerEventProcessor, TwoInternedStringsDifferentProducersSameKey) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1 = CreateInternedStringEvent(kKey1, "string1");
  ProducerCaptureEvent event2 = CreateInternedStringEvent(kKey1, "string2");

  ClientCaptureEvent client_capture_event1;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event1));
  producer_event_processor->ProcessEvent(1, std::move(event1));

  ClientCaptureEvent client_capture_event2;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event2));
  producer_event_processor->ProcessEvent(2, std::move(event2));

  ASSERT_EQ(client_capture_event1.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(client_capture_event2.event_case(), ClientCaptureEvent::kInternedString);

  const InternedString& actual_interned_string1 = client_capture_event1.interned_string();
  const InternedString& actual_interned_string2 = client_capture_event2.interned_string();

  EXPECT_NE(actual_interned_string1.key(), actual_interned_string2.key());

  EXPECT_NE(actual_interned_string1.key(), orbit_grpc_protos::kInvalidInternId);
  EXPECT_EQ(actual_interned_string1.intern(), "string1");

  EXPECT_NE(actual_interned_string1.key(), orbit_grpc_protos::kInvalidInternId);
  EXPECT_EQ(actual_interned_string2.intern(), "string2");
}

TEST(ProducerEventProcessor, TwoInternedStringsDifferentProducersSameIntern) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  constexpr uint64_t kProducer1StringKey = kKey1;
  constexpr uint64_t kProducer2StringKey = kKey2;

  ProducerCaptureEvent event1 = CreateInternedStringEvent(kProducer1StringKey, "string");
  ProducerCaptureEvent event2 = CreateInternedStringEvent(kProducer2StringKey, "string");

  ClientCaptureEvent client_capture_event;
  // We expect only one call here because we have same string.
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));
  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(2, std::move(event2));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kInternedString);
  const InternedString& actual_interned_string = client_capture_event.interned_string();

  EXPECT_NE(actual_interned_string.key(), orbit_grpc_protos::kInvalidInternId);
  EXPECT_EQ(actual_interned_string.intern(), "string");

  // Now check that the both producer's strings are still tracked by
  // ProducerEventProcessor
  testing::Mock::VerifyAndClearExpectations(&collector);

  ProducerCaptureEvent gpu_queue_submission_event1;
  GpuQueueSubmission* gpu_queue_submission1 =
      gpu_queue_submission_event1.mutable_gpu_queue_submission();
  gpu_queue_submission1->set_num_begin_markers(kNumBeginMarkers1);
  gpu_queue_submission1->mutable_meta_info()->set_tid(kTid1);
  gpu_queue_submission1->mutable_meta_info()->set_pre_submission_cpu_timestamp(kTimestampNs1);
  gpu_queue_submission1->mutable_meta_info()->set_post_submission_cpu_timestamp(kTimestampNs2);
  GpuDebugMarker* completed_marker1 = gpu_queue_submission1->add_completed_markers();
  completed_marker1->set_depth(kDepth1);
  completed_marker1->set_end_gpu_timestamp_ns(kTimestampNs1);
  completed_marker1->set_text_key(kProducer1StringKey);

  ProducerCaptureEvent gpu_queue_submission_event2;
  GpuQueueSubmission* gpu_queue_submission2 =
      gpu_queue_submission_event2.mutable_gpu_queue_submission();
  gpu_queue_submission2->set_num_begin_markers(kNumBeginMarkers2);
  gpu_queue_submission2->mutable_meta_info()->set_tid(kTid2);
  gpu_queue_submission2->mutable_meta_info()->set_pre_submission_cpu_timestamp(kTimestampNs1);
  gpu_queue_submission2->mutable_meta_info()->set_post_submission_cpu_timestamp(kTimestampNs2);
  GpuDebugMarker* completed_marker2 = gpu_queue_submission2->add_completed_markers();
  completed_marker2->set_depth(kDepth2);
  completed_marker2->set_end_gpu_timestamp_ns(kTimestampNs2);
  completed_marker2->set_text_key(kProducer2StringKey);

  ClientCaptureEvent client_capture_event1;
  ClientCaptureEvent client_capture_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(2)
      .WillOnce(SaveArg<0>(&client_capture_event1))
      .WillOnce(SaveArg<0>(&client_capture_event2));
  producer_event_processor->ProcessEvent(1, std::move(gpu_queue_submission_event1));
  producer_event_processor->ProcessEvent(2, std::move(gpu_queue_submission_event2));

  ASSERT_EQ(client_capture_event1.event_case(), ClientCaptureEvent::kGpuQueueSubmission);
  ASSERT_EQ(client_capture_event2.event_case(), ClientCaptureEvent::kGpuQueueSubmission);

  const GpuQueueSubmission& submission1 = client_capture_event1.gpu_queue_submission();
  const GpuQueueSubmission& submission2 = client_capture_event2.gpu_queue_submission();

  EXPECT_EQ(submission1.num_begin_markers(), kNumBeginMarkers1);
  EXPECT_EQ(submission1.meta_info().tid(), kTid1);
  EXPECT_EQ(submission1.meta_info().pre_submission_cpu_timestamp(), kTimestampNs1);
  EXPECT_EQ(submission1.meta_info().post_submission_cpu_timestamp(), kTimestampNs2);
  ASSERT_EQ(submission1.completed_markers_size(), 1);
  EXPECT_EQ(submission1.completed_markers()[0].depth(), kDepth1);
  EXPECT_EQ(submission1.completed_markers()[0].end_gpu_timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(submission1.completed_markers()[0].text_key(), actual_interned_string.key());

  EXPECT_EQ(submission2.num_begin_markers(), kNumBeginMarkers2);
  EXPECT_EQ(submission2.meta_info().tid(), kTid2);
  EXPECT_EQ(submission2.meta_info().pre_submission_cpu_timestamp(), kTimestampNs1);
  EXPECT_EQ(submission2.meta_info().post_submission_cpu_timestamp(), kTimestampNs2);
  ASSERT_EQ(submission2.completed_markers_size(), 1);
  EXPECT_EQ(submission2.completed_markers()[0].depth(), kDepth2);
  EXPECT_EQ(submission2.completed_markers()[0].end_gpu_timestamp_ns(), kTimestampNs2);
  EXPECT_EQ(submission2.completed_markers()[0].text_key(), actual_interned_string.key());
}

TEST(ProducerEventProcessor, FullCallstackSampleDifferentCallstacks) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  FullCallstackSample* full_callstack_sample1 = event1.mutable_full_callstack_sample();
  full_callstack_sample1->set_pid(kPid1);
  full_callstack_sample1->set_tid(kTid1);
  full_callstack_sample1->set_timestamp_ns(kTimestampNs1);
  Callstack* callstack1 = full_callstack_sample1->mutable_callstack();
  callstack1->add_pcs(1);
  callstack1->add_pcs(2);
  callstack1->add_pcs(3);
  callstack1->add_pcs(4);
  callstack1->set_type(Callstack::kComplete);

  ProducerCaptureEvent event2;
  FullCallstackSample* full_callstack_sample2 = event2.mutable_full_callstack_sample();
  full_callstack_sample2->set_pid(kPid2);
  full_callstack_sample2->set_tid(kTid2);
  full_callstack_sample2->set_timestamp_ns(kTimestampNs2);
  Callstack* callstack2 = full_callstack_sample2->mutable_callstack();
  callstack2->add_pcs(5);
  callstack2->add_pcs(6);
  callstack2->add_pcs(7);
  callstack2->add_pcs(8);
  callstack2->set_type(Callstack::kComplete);

  ClientCaptureEvent interned_callstack_event1;
  ClientCaptureEvent callstack_sample_event1;
  ClientCaptureEvent interned_callstack_event2;
  ClientCaptureEvent callstack_sample_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(4)
      .WillOnce(SaveArg<0>(&interned_callstack_event1))
      .WillOnce(SaveArg<0>(&callstack_sample_event1))
      .WillOnce(SaveArg<0>(&interned_callstack_event2))
      .WillOnce(SaveArg<0>(&callstack_sample_event2));

  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(1, std::move(event2));

  ASSERT_EQ(interned_callstack_event1.event_case(), ClientCaptureEvent::kInternedCallstack);
  ASSERT_EQ(interned_callstack_event2.event_case(), ClientCaptureEvent::kInternedCallstack);
  ASSERT_EQ(callstack_sample_event1.event_case(), ClientCaptureEvent::kCallstackSample);
  ASSERT_EQ(callstack_sample_event2.event_case(), ClientCaptureEvent::kCallstackSample);

  const InternedCallstack& interned_callstack1 = interned_callstack_event1.interned_callstack();
  EXPECT_NE(interned_callstack1.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(interned_callstack1.intern().pcs_size(), 4);
  EXPECT_EQ(interned_callstack1.intern().pcs(0), 1);
  EXPECT_EQ(interned_callstack1.intern().pcs(1), 2);
  EXPECT_EQ(interned_callstack1.intern().pcs(2), 3);
  EXPECT_EQ(interned_callstack1.intern().pcs(3), 4);
  EXPECT_EQ(interned_callstack1.intern().type(), Callstack::kComplete);

  const InternedCallstack& interned_callstack2 = interned_callstack_event2.interned_callstack();
  EXPECT_NE(interned_callstack2.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(interned_callstack2.intern().pcs_size(), 4);
  EXPECT_EQ(interned_callstack2.intern().pcs(0), 5);
  EXPECT_EQ(interned_callstack2.intern().pcs(1), 6);
  EXPECT_EQ(interned_callstack2.intern().pcs(2), 7);
  EXPECT_EQ(interned_callstack2.intern().pcs(3), 8);
  EXPECT_EQ(interned_callstack2.intern().type(), Callstack::kComplete);

  const CallstackSample& callstack_sample1 = callstack_sample_event1.callstack_sample();
  EXPECT_EQ(callstack_sample1.pid(), kPid1);
  EXPECT_EQ(callstack_sample1.tid(), kTid1);
  EXPECT_EQ(callstack_sample1.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(callstack_sample1.callstack_id(), interned_callstack1.key());

  const CallstackSample& callstack_sample2 = callstack_sample_event2.callstack_sample();
  EXPECT_EQ(callstack_sample2.pid(), kPid2);
  EXPECT_EQ(callstack_sample2.tid(), kTid2);
  EXPECT_EQ(callstack_sample2.timestamp_ns(), kTimestampNs2);
  EXPECT_EQ(callstack_sample2.callstack_id(), interned_callstack2.key());
}

TEST(ProducerEventProcessor, FullCallstackSampleSameFramesDifferentTypes) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  FullCallstackSample* full_callstack_sample1 = event1.mutable_full_callstack_sample();
  full_callstack_sample1->set_pid(kPid1);
  full_callstack_sample1->set_tid(kTid1);
  full_callstack_sample1->set_timestamp_ns(kTimestampNs1);
  Callstack* callstack1 = full_callstack_sample1->mutable_callstack();
  callstack1->add_pcs(1);
  callstack1->add_pcs(2);
  callstack1->add_pcs(3);
  callstack1->add_pcs(4);
  callstack1->set_type(Callstack::kComplete);

  ProducerCaptureEvent event2;
  FullCallstackSample* full_callstack_sample2 = event2.mutable_full_callstack_sample();
  full_callstack_sample2->set_pid(kPid2);
  full_callstack_sample2->set_tid(kTid2);
  full_callstack_sample2->set_timestamp_ns(kTimestampNs2);
  Callstack* callstack2 = full_callstack_sample2->mutable_callstack();
  callstack2->add_pcs(1);
  callstack2->add_pcs(2);
  callstack2->add_pcs(3);
  callstack2->add_pcs(4);
  callstack2->set_type(Callstack::kDwarfUnwindingError);

  ClientCaptureEvent interned_callstack_event1;
  ClientCaptureEvent callstack_sample_event1;
  ClientCaptureEvent interned_callstack_event2;
  ClientCaptureEvent callstack_sample_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(4)
      .WillOnce(SaveArg<0>(&interned_callstack_event1))
      .WillOnce(SaveArg<0>(&callstack_sample_event1))
      .WillOnce(SaveArg<0>(&interned_callstack_event2))
      .WillOnce(SaveArg<0>(&callstack_sample_event2));

  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(1, std::move(event2));

  ASSERT_EQ(interned_callstack_event1.event_case(), ClientCaptureEvent::kInternedCallstack);
  ASSERT_EQ(interned_callstack_event2.event_case(), ClientCaptureEvent::kInternedCallstack);
  ASSERT_EQ(callstack_sample_event1.event_case(), ClientCaptureEvent::kCallstackSample);
  ASSERT_EQ(callstack_sample_event2.event_case(), ClientCaptureEvent::kCallstackSample);

  const InternedCallstack& interned_callstack1 = interned_callstack_event1.interned_callstack();
  EXPECT_NE(interned_callstack1.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(interned_callstack1.intern().pcs_size(), 4);
  EXPECT_EQ(interned_callstack1.intern().pcs(0), 1);
  EXPECT_EQ(interned_callstack1.intern().pcs(1), 2);
  EXPECT_EQ(interned_callstack1.intern().pcs(2), 3);
  EXPECT_EQ(interned_callstack1.intern().pcs(3), 4);
  EXPECT_EQ(interned_callstack1.intern().type(), Callstack::kComplete);

  const InternedCallstack& interned_callstack2 = interned_callstack_event2.interned_callstack();
  EXPECT_NE(interned_callstack2.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(interned_callstack2.intern().pcs_size(), 4);
  EXPECT_EQ(interned_callstack2.intern().pcs(0), 1);
  EXPECT_EQ(interned_callstack2.intern().pcs(1), 2);
  EXPECT_EQ(interned_callstack2.intern().pcs(2), 3);
  EXPECT_EQ(interned_callstack2.intern().pcs(3), 4);
  EXPECT_EQ(interned_callstack2.intern().type(), Callstack::kDwarfUnwindingError);

  const CallstackSample& callstack_sample1 = callstack_sample_event1.callstack_sample();
  EXPECT_EQ(callstack_sample1.pid(), kPid1);
  EXPECT_EQ(callstack_sample1.tid(), kTid1);
  EXPECT_EQ(callstack_sample1.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(callstack_sample1.callstack_id(), interned_callstack1.key());

  const CallstackSample& callstack_sample2 = callstack_sample_event2.callstack_sample();
  EXPECT_EQ(callstack_sample2.pid(), kPid2);
  EXPECT_EQ(callstack_sample2.tid(), kTid2);
  EXPECT_EQ(callstack_sample2.timestamp_ns(), kTimestampNs2);
  EXPECT_EQ(callstack_sample2.callstack_id(), interned_callstack2.key());
}

TEST(ProducerEventProcessor, FullCallstackSamplesSameCallstack) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  FullCallstackSample* full_callstack_sample1 = event1.mutable_full_callstack_sample();
  full_callstack_sample1->set_pid(kPid1);
  full_callstack_sample1->set_tid(kTid1);
  full_callstack_sample1->set_timestamp_ns(kTimestampNs1);
  Callstack* callstack1 = full_callstack_sample1->mutable_callstack();
  callstack1->add_pcs(1);
  callstack1->add_pcs(2);
  callstack1->add_pcs(3);
  callstack1->add_pcs(4);
  callstack1->set_type(Callstack::kComplete);

  ProducerCaptureEvent event2;
  FullCallstackSample* full_callstack_sample2 = event2.mutable_full_callstack_sample();
  full_callstack_sample2->set_pid(kPid2);
  full_callstack_sample2->set_tid(kTid2);
  full_callstack_sample2->set_timestamp_ns(kTimestampNs2);
  Callstack* callstack2 = full_callstack_sample2->mutable_callstack();
  callstack2->add_pcs(1);
  callstack2->add_pcs(2);
  callstack2->add_pcs(3);
  callstack2->add_pcs(4);
  callstack2->set_type(Callstack::kComplete);

  ClientCaptureEvent interned_callstack_event1;
  ClientCaptureEvent callstack_sample_event1;
  ClientCaptureEvent callstack_sample_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(3)
      .WillOnce(SaveArg<0>(&interned_callstack_event1))
      .WillOnce(SaveArg<0>(&callstack_sample_event1))
      .WillOnce(SaveArg<0>(&callstack_sample_event2));

  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(1, std::move(event2));

  ASSERT_EQ(interned_callstack_event1.event_case(), ClientCaptureEvent::kInternedCallstack);
  ASSERT_EQ(callstack_sample_event1.event_case(), ClientCaptureEvent::kCallstackSample);
  ASSERT_EQ(callstack_sample_event2.event_case(), ClientCaptureEvent::kCallstackSample);

  const InternedCallstack& interned_callstack1 = interned_callstack_event1.interned_callstack();
  EXPECT_NE(interned_callstack1.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(interned_callstack1.intern().pcs_size(), 4);
  EXPECT_EQ(interned_callstack1.intern().pcs(0), 1);
  EXPECT_EQ(interned_callstack1.intern().pcs(1), 2);
  EXPECT_EQ(interned_callstack1.intern().pcs(2), 3);
  EXPECT_EQ(interned_callstack1.intern().pcs(3), 4);
  EXPECT_EQ(interned_callstack1.intern().type(), Callstack::kComplete);

  const CallstackSample& callstack_sample1 = callstack_sample_event1.callstack_sample();
  EXPECT_EQ(callstack_sample1.pid(), kPid1);
  EXPECT_EQ(callstack_sample1.tid(), kTid1);
  EXPECT_EQ(callstack_sample1.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(callstack_sample1.callstack_id(), interned_callstack1.key());

  const CallstackSample& callstack_sample2 = callstack_sample_event2.callstack_sample();
  EXPECT_EQ(callstack_sample2.pid(), kPid2);
  EXPECT_EQ(callstack_sample2.tid(), kTid2);
  EXPECT_EQ(callstack_sample2.timestamp_ns(), kTimestampNs2);
  EXPECT_EQ(callstack_sample2.callstack_id(), interned_callstack1.key());
}

TEST(ProducerEventProcessor, FullTracepointEventsDifferentTracepoints) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  FullTracepointEvent* full_tracepoint_event1 = event1.mutable_full_tracepoint_event();
  full_tracepoint_event1->set_pid(kPid1);
  full_tracepoint_event1->set_tid(kTid1);
  full_tracepoint_event1->set_timestamp_ns(kTimestampNs1);
  full_tracepoint_event1->mutable_tracepoint_info()->set_category("category1");
  full_tracepoint_event1->mutable_tracepoint_info()->set_name("name1");

  ProducerCaptureEvent event2;
  FullTracepointEvent* full_tracepoint_event2 = event2.mutable_full_tracepoint_event();
  full_tracepoint_event2->set_pid(kPid2);
  full_tracepoint_event2->set_tid(kTid2);
  full_tracepoint_event2->set_timestamp_ns(kTimestampNs2);
  full_tracepoint_event2->mutable_tracepoint_info()->set_category("category2");
  full_tracepoint_event2->mutable_tracepoint_info()->set_name("name2");

  ClientCaptureEvent interned_tracepoint_info_event1;
  ClientCaptureEvent client_tracepoint_event1;
  ClientCaptureEvent interned_tracepoint_info_event2;
  ClientCaptureEvent client_tracepoint_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(4)
      .WillOnce(SaveArg<0>(&interned_tracepoint_info_event1))
      .WillOnce(SaveArg<0>(&client_tracepoint_event1))
      .WillOnce(SaveArg<0>(&interned_tracepoint_info_event2))
      .WillOnce(SaveArg<0>(&client_tracepoint_event2));

  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(1, std::move(event2));

  ASSERT_EQ(interned_tracepoint_info_event1.event_case(),
            ClientCaptureEvent::kInternedTracepointInfo);
  ASSERT_EQ(interned_tracepoint_info_event2.event_case(),
            ClientCaptureEvent::kInternedTracepointInfo);
  ASSERT_EQ(client_tracepoint_event1.event_case(), ClientCaptureEvent::kTracepointEvent);
  ASSERT_EQ(client_tracepoint_event2.event_case(), ClientCaptureEvent::kTracepointEvent);

  const InternedTracepointInfo& interned_tracepoint_info1 =
      interned_tracepoint_info_event1.interned_tracepoint_info();
  EXPECT_NE(interned_tracepoint_info1.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(interned_tracepoint_info1.intern().name(), "name1");
  EXPECT_EQ(interned_tracepoint_info1.intern().category(), "category1");

  const InternedTracepointInfo& interned_tracepoint_info2 =
      interned_tracepoint_info_event2.interned_tracepoint_info();
  EXPECT_NE(interned_tracepoint_info2.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(interned_tracepoint_info2.intern().name(), "name2");
  EXPECT_EQ(interned_tracepoint_info2.intern().category(), "category2");

  const TracepointEvent& tracepoint_event1 = client_tracepoint_event1.tracepoint_event();
  EXPECT_EQ(tracepoint_event1.pid(), kPid1);
  EXPECT_EQ(tracepoint_event1.tid(), kTid1);
  EXPECT_EQ(tracepoint_event1.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(tracepoint_event1.tracepoint_info_key(), interned_tracepoint_info1.key());

  const TracepointEvent& tracepoint_event2 = client_tracepoint_event2.tracepoint_event();
  EXPECT_EQ(tracepoint_event2.pid(), kPid2);
  EXPECT_EQ(tracepoint_event2.tid(), kTid2);
  EXPECT_EQ(tracepoint_event2.timestamp_ns(), kTimestampNs2);
  EXPECT_EQ(tracepoint_event2.tracepoint_info_key(), interned_tracepoint_info2.key());
}

TEST(ProducerEventProcessor, FullTracepointEventsSameTracepoint) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  FullTracepointEvent* full_tracepoint_event1 = event1.mutable_full_tracepoint_event();
  full_tracepoint_event1->set_pid(kPid1);
  full_tracepoint_event1->set_tid(kTid1);
  full_tracepoint_event1->set_timestamp_ns(kTimestampNs1);
  full_tracepoint_event1->mutable_tracepoint_info()->set_category("category1");
  full_tracepoint_event1->mutable_tracepoint_info()->set_name("name1");

  ProducerCaptureEvent event2;
  FullTracepointEvent* full_tracepoint_event2 = event2.mutable_full_tracepoint_event();
  full_tracepoint_event2->set_pid(kPid2);
  full_tracepoint_event2->set_tid(kTid2);
  full_tracepoint_event2->set_timestamp_ns(kTimestampNs2);
  full_tracepoint_event2->mutable_tracepoint_info()->set_category("category1");
  full_tracepoint_event2->mutable_tracepoint_info()->set_name("name1");

  ClientCaptureEvent interned_tracepoint_info_event1;
  ClientCaptureEvent client_tracepoint_event1;
  ClientCaptureEvent client_tracepoint_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(3)
      .WillOnce(SaveArg<0>(&interned_tracepoint_info_event1))
      .WillOnce(SaveArg<0>(&client_tracepoint_event1))
      .WillOnce(SaveArg<0>(&client_tracepoint_event2));

  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(1, std::move(event2));

  ASSERT_EQ(interned_tracepoint_info_event1.event_case(),
            ClientCaptureEvent::kInternedTracepointInfo);
  ASSERT_EQ(client_tracepoint_event1.event_case(), ClientCaptureEvent::kTracepointEvent);
  ASSERT_EQ(client_tracepoint_event2.event_case(), ClientCaptureEvent::kTracepointEvent);

  const InternedTracepointInfo& interned_tracepoint_info1 =
      interned_tracepoint_info_event1.interned_tracepoint_info();
  EXPECT_NE(interned_tracepoint_info1.key(), orbit_grpc_protos::kInvalidInternId);
  ASSERT_EQ(interned_tracepoint_info1.intern().name(), "name1");
  EXPECT_EQ(interned_tracepoint_info1.intern().category(), "category1");

  const TracepointEvent& tracepoint_event1 = client_tracepoint_event1.tracepoint_event();
  EXPECT_EQ(tracepoint_event1.pid(), kPid1);
  EXPECT_EQ(tracepoint_event1.tid(), kTid1);
  EXPECT_EQ(tracepoint_event1.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(tracepoint_event1.tracepoint_info_key(), interned_tracepoint_info1.key());

  const TracepointEvent& tracepoint_event2 = client_tracepoint_event2.tracepoint_event();
  EXPECT_EQ(tracepoint_event2.pid(), kPid2);
  EXPECT_EQ(tracepoint_event2.tid(), kTid2);
  EXPECT_EQ(tracepoint_event2.timestamp_ns(), kTimestampNs2);
  EXPECT_EQ(tracepoint_event2.tracepoint_info_key(), interned_tracepoint_info1.key());
}

TEST(ProducerEventProcessor, FunctionCallSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  {
    FunctionCall* function_call = event1.mutable_function_call();
    function_call->set_pid(kPid1);
    function_call->set_tid(kTid1);
    function_call->set_function_id(kFunctionId1);
    function_call->set_depth(kDepth1);
    function_call->set_duration_ns(kDurationNs1);
    function_call->set_end_timestamp_ns(kTimestampNs1);
    function_call->set_return_value(42);
    function_call->add_registers(42);
    function_call->add_registers(2);
    function_call->add_registers(3);
  }

  ProducerCaptureEvent event2;
  {
    FunctionCall* function_call = event2.mutable_function_call();
    function_call->set_pid(kPid2);
    function_call->set_tid(kTid2);
    function_call->set_function_id(kFunctionId2);
    function_call->set_depth(kDepth2);
    function_call->set_duration_ns(kDurationNs2);
    function_call->set_end_timestamp_ns(kTimestampNs2);
    function_call->set_return_value(42);
    function_call->add_registers(42);
    function_call->add_registers(21);
    function_call->add_registers(31);
  }

  ClientCaptureEvent function_call_event1;
  ClientCaptureEvent function_call_event2;
  EXPECT_CALL(collector, AddEvent)
      .Times(2)
      .WillOnce(SaveArg<0>(&function_call_event1))
      .WillOnce(SaveArg<0>(&function_call_event2));

  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(1, std::move(event2));

  ASSERT_EQ(function_call_event1.event_case(), ClientCaptureEvent::kFunctionCall);
  ASSERT_EQ(function_call_event1.event_case(), ClientCaptureEvent::kFunctionCall);

  {
    const FunctionCall& function_call = function_call_event1.function_call();
    EXPECT_EQ(function_call.pid(), kPid1);
    EXPECT_EQ(function_call.tid(), kTid1);
    EXPECT_EQ(function_call.function_id(), kFunctionId1);
    EXPECT_EQ(function_call.duration_ns(), kDurationNs1);
    EXPECT_EQ(function_call.end_timestamp_ns(), kTimestampNs1);
    EXPECT_EQ(function_call.depth(), kDepth1);
    EXPECT_EQ(function_call.return_value(), 42);
    ASSERT_EQ(function_call.registers_size(), 3);
    EXPECT_EQ(function_call.registers(0), 42);
    EXPECT_EQ(function_call.registers(1), 2);
    EXPECT_EQ(function_call.registers(2), 3);
  }

  {
    const FunctionCall& function_call = function_call_event2.function_call();
    EXPECT_EQ(function_call.pid(), kPid2);
    EXPECT_EQ(function_call.tid(), kTid2);
    EXPECT_EQ(function_call.function_id(), kFunctionId2);
    EXPECT_EQ(function_call.duration_ns(), kDurationNs2);
    EXPECT_EQ(function_call.end_timestamp_ns(), kTimestampNs2);
    EXPECT_EQ(function_call.depth(), kDepth2);
    EXPECT_EQ(function_call.return_value(), 42);
    ASSERT_EQ(function_call.registers_size(), 3);
    EXPECT_EQ(function_call.registers(0), 42);
    EXPECT_EQ(function_call.registers(1), 21);
    EXPECT_EQ(function_call.registers(2), 31);
  }
}

TEST(ProducerEventProcessor, FullGpuJobDifferentTimelines) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  {
    FullGpuJob* gpu_job = event1.mutable_full_gpu_job();
    gpu_job->set_pid(kPid1);
    gpu_job->set_tid(kTid1);
    gpu_job->set_context(kGpuJobContext1);
    gpu_job->set_seqno(kSeqNo1);
    gpu_job->set_depth(kDepth1);
    gpu_job->set_amdgpu_cs_ioctl_time_ns(kTimestampNs1);
    gpu_job->set_amdgpu_sched_run_job_time_ns(kTimestampNs1 + 1);
    gpu_job->set_gpu_hardware_start_time_ns(kTimestampNs1 + 2);
    gpu_job->set_dma_fence_signaled_time_ns(kTimestampNs1 + 3);
    gpu_job->set_timeline("timeline1");
  }

  ProducerCaptureEvent event2;
  {
    FullGpuJob* gpu_job = event2.mutable_full_gpu_job();
    gpu_job->set_pid(kPid2);
    gpu_job->set_tid(kTid2);
    gpu_job->set_context(kGpuJobContext2);
    gpu_job->set_seqno(kSeqNo2);
    gpu_job->set_depth(kDepth2);
    gpu_job->set_amdgpu_cs_ioctl_time_ns(kTimestampNs2);
    gpu_job->set_amdgpu_sched_run_job_time_ns(kTimestampNs2 + 1);
    gpu_job->set_gpu_hardware_start_time_ns(kTimestampNs2 + 2);
    gpu_job->set_dma_fence_signaled_time_ns(kTimestampNs2 + 3);
    gpu_job->set_timeline("timeline2");
  }

  ClientCaptureEvent interned_string1;
  ClientCaptureEvent gpu_job_event1;
  ClientCaptureEvent interned_string2;
  ClientCaptureEvent gpu_job_event2;

  EXPECT_CALL(collector, AddEvent)
      .Times(4)
      .WillOnce(SaveArg<0>(&interned_string1))
      .WillOnce(SaveArg<0>(&gpu_job_event1))
      .WillOnce(SaveArg<0>(&interned_string2))
      .WillOnce(SaveArg<0>(&gpu_job_event2));

  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(1, std::move(event2));

  ASSERT_EQ(interned_string1.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(interned_string2.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(gpu_job_event1.event_case(), ClientCaptureEvent::kGpuJob);
  ASSERT_EQ(gpu_job_event2.event_case(), ClientCaptureEvent::kGpuJob);

  {
    const InternedString& interned_string = interned_string1.interned_string();
    EXPECT_NE(interned_string.key(), orbit_grpc_protos::kInvalidInternId);
    EXPECT_EQ(interned_string.intern(), "timeline1");
  }

  {
    const InternedString& interned_string = interned_string2.interned_string();
    EXPECT_NE(interned_string.key(), orbit_grpc_protos::kInvalidInternId);
    EXPECT_EQ(interned_string.intern(), "timeline2");
  }

  {
    const GpuJob& gpu_job = gpu_job_event1.gpu_job();
    EXPECT_EQ(gpu_job.pid(), kPid1);
    EXPECT_EQ(gpu_job.tid(), kTid1);
    EXPECT_EQ(gpu_job.context(), kGpuJobContext1);
    EXPECT_EQ(gpu_job.seqno(), kSeqNo1);
    EXPECT_EQ(gpu_job.depth(), kDepth1);
    EXPECT_EQ(gpu_job.amdgpu_cs_ioctl_time_ns(), kTimestampNs1);
    EXPECT_EQ(gpu_job.amdgpu_sched_run_job_time_ns(), kTimestampNs1 + 1);
    EXPECT_EQ(gpu_job.gpu_hardware_start_time_ns(), kTimestampNs1 + 2);
    EXPECT_EQ(gpu_job.dma_fence_signaled_time_ns(), kTimestampNs1 + 3);
    EXPECT_EQ(gpu_job.timeline_key(), interned_string1.interned_string().key());
  }

  {
    const GpuJob& gpu_job = gpu_job_event2.gpu_job();
    EXPECT_EQ(gpu_job.pid(), kPid2);
    EXPECT_EQ(gpu_job.tid(), kTid2);
    EXPECT_EQ(gpu_job.context(), kGpuJobContext2);
    EXPECT_EQ(gpu_job.seqno(), kSeqNo2);
    EXPECT_EQ(gpu_job.depth(), kDepth2);
    EXPECT_EQ(gpu_job.amdgpu_cs_ioctl_time_ns(), kTimestampNs2);
    EXPECT_EQ(gpu_job.amdgpu_sched_run_job_time_ns(), kTimestampNs2 + 1);
    EXPECT_EQ(gpu_job.gpu_hardware_start_time_ns(), kTimestampNs2 + 2);
    EXPECT_EQ(gpu_job.dma_fence_signaled_time_ns(), kTimestampNs2 + 3);
    EXPECT_EQ(gpu_job.timeline_key(), interned_string2.interned_string().key());
  }
}

TEST(ProducerEventProcessor, FullGpuJobSameTimeline) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  {
    FullGpuJob* gpu_job = event1.mutable_full_gpu_job();
    gpu_job->set_pid(kPid1);
    gpu_job->set_tid(kTid1);
    gpu_job->set_context(kGpuJobContext1);
    gpu_job->set_seqno(kSeqNo1);
    gpu_job->set_depth(kDepth1);
    gpu_job->set_amdgpu_cs_ioctl_time_ns(kTimestampNs1);
    gpu_job->set_amdgpu_sched_run_job_time_ns(kTimestampNs1 + 1);
    gpu_job->set_gpu_hardware_start_time_ns(kTimestampNs1 + 2);
    gpu_job->set_dma_fence_signaled_time_ns(kTimestampNs1 + 3);
    gpu_job->set_timeline("timeline1");
  }

  ProducerCaptureEvent event2;
  {
    FullGpuJob* gpu_job = event2.mutable_full_gpu_job();
    gpu_job->set_pid(kPid2);
    gpu_job->set_tid(kTid2);
    gpu_job->set_context(kGpuJobContext2);
    gpu_job->set_seqno(kSeqNo2);
    gpu_job->set_depth(kDepth2);
    gpu_job->set_amdgpu_cs_ioctl_time_ns(kTimestampNs2);
    gpu_job->set_amdgpu_sched_run_job_time_ns(kTimestampNs2 + 1);
    gpu_job->set_gpu_hardware_start_time_ns(kTimestampNs2 + 2);
    gpu_job->set_dma_fence_signaled_time_ns(kTimestampNs2 + 3);
    gpu_job->set_timeline("timeline1");
  }

  ClientCaptureEvent interned_string_event;
  ClientCaptureEvent gpu_job_event1;
  ClientCaptureEvent gpu_job_event2;

  EXPECT_CALL(collector, AddEvent)
      .Times(3)
      .WillOnce(SaveArg<0>(&interned_string_event))
      .WillOnce(SaveArg<0>(&gpu_job_event1))
      .WillOnce(SaveArg<0>(&gpu_job_event2));

  producer_event_processor->ProcessEvent(1, std::move(event1));
  producer_event_processor->ProcessEvent(1, std::move(event2));

  ASSERT_EQ(interned_string_event.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(gpu_job_event1.event_case(), ClientCaptureEvent::kGpuJob);
  ASSERT_EQ(gpu_job_event2.event_case(), ClientCaptureEvent::kGpuJob);

  {
    const InternedString& interned_string = interned_string_event.interned_string();
    EXPECT_NE(interned_string.key(), orbit_grpc_protos::kInvalidInternId);
    EXPECT_EQ(interned_string.intern(), "timeline1");
  }

  {
    const GpuJob& gpu_job = gpu_job_event1.gpu_job();
    EXPECT_EQ(gpu_job.pid(), kPid1);
    EXPECT_EQ(gpu_job.tid(), kTid1);
    EXPECT_EQ(gpu_job.context(), kGpuJobContext1);
    EXPECT_EQ(gpu_job.seqno(), kSeqNo1);
    EXPECT_EQ(gpu_job.depth(), kDepth1);
    EXPECT_EQ(gpu_job.amdgpu_cs_ioctl_time_ns(), kTimestampNs1);
    EXPECT_EQ(gpu_job.amdgpu_sched_run_job_time_ns(), kTimestampNs1 + 1);
    EXPECT_EQ(gpu_job.gpu_hardware_start_time_ns(), kTimestampNs1 + 2);
    EXPECT_EQ(gpu_job.dma_fence_signaled_time_ns(), kTimestampNs1 + 3);
    EXPECT_EQ(gpu_job.timeline_key(), interned_string_event.interned_string().key());
  }

  {
    const GpuJob& gpu_job = gpu_job_event2.gpu_job();
    EXPECT_EQ(gpu_job.pid(), kPid2);
    EXPECT_EQ(gpu_job.tid(), kTid2);
    EXPECT_EQ(gpu_job.context(), kGpuJobContext2);
    EXPECT_EQ(gpu_job.seqno(), kSeqNo2);
    EXPECT_EQ(gpu_job.depth(), kDepth2);
    EXPECT_EQ(gpu_job.amdgpu_cs_ioctl_time_ns(), kTimestampNs2);
    EXPECT_EQ(gpu_job.amdgpu_sched_run_job_time_ns(), kTimestampNs2 + 1);
    EXPECT_EQ(gpu_job.gpu_hardware_start_time_ns(), kTimestampNs2 + 2);
    EXPECT_EQ(gpu_job.dma_fence_signaled_time_ns(), kTimestampNs2 + 3);
    EXPECT_EQ(gpu_job.timeline_key(), interned_string_event.interned_string().key());
  }
}

TEST(ProducerEventProcessor, GpuQueueSubmissionSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_string_event1 = CreateInternedStringEvent(kKey1, "debug marker1");
  ProducerCaptureEvent producer_string_event2 = CreateInternedStringEvent(kKey2, "debug marker2");

  ProducerCaptureEvent producer_gpu_submission_event;
  {
    GpuQueueSubmission* gpu_queue_submission =
        producer_gpu_submission_event.mutable_gpu_queue_submission();
    gpu_queue_submission->mutable_meta_info()->set_tid(kTid1);
    gpu_queue_submission->mutable_meta_info()->set_pre_submission_cpu_timestamp(kTimestampNs1);
    gpu_queue_submission->mutable_meta_info()->set_post_submission_cpu_timestamp(kTimestampNs1 + 1);
    gpu_queue_submission->set_num_begin_markers(2);

    {
      GpuSubmitInfo* submit_info = gpu_queue_submission->add_submit_infos();
      {
        GpuCommandBuffer* command_buffer = submit_info->add_command_buffers();
        command_buffer->set_begin_gpu_timestamp_ns(kTimestampNs1 + 10);
        command_buffer->set_end_gpu_timestamp_ns(kTimestampNs1 + 11);
      }
      {
        GpuCommandBuffer* command_buffer = submit_info->add_command_buffers();
        command_buffer->set_begin_gpu_timestamp_ns(kTimestampNs1 + 20);
        command_buffer->set_end_gpu_timestamp_ns(kTimestampNs1 + 21);
      }
    }

    {
      GpuSubmitInfo* submit_info = gpu_queue_submission->add_submit_infos();
      {
        GpuCommandBuffer* command_buffer = submit_info->add_command_buffers();
        command_buffer->set_begin_gpu_timestamp_ns(kTimestampNs1 + 30);
        command_buffer->set_end_gpu_timestamp_ns(kTimestampNs1 + 31);
      }
    }

    {
      GpuDebugMarker* debug_marker = gpu_queue_submission->add_completed_markers();
      debug_marker->mutable_begin_marker()->set_gpu_timestamp_ns(kTimestampNs1 + 40);
      debug_marker->mutable_begin_marker()->mutable_meta_info()->set_tid(kTid1 + 1);
      debug_marker->mutable_begin_marker()->mutable_meta_info()->set_pre_submission_cpu_timestamp(
          kTimestampNs1 + 100);
      debug_marker->mutable_begin_marker()->mutable_meta_info()->set_post_submission_cpu_timestamp(
          kTimestampNs1 + 101);
      debug_marker->set_text_key(kKey1);
      debug_marker->set_depth(kDepth1);
      debug_marker->set_end_gpu_timestamp_ns(kTimestampNs1 + 50);

      debug_marker->mutable_color()->set_alpha(kAlpha1);
      debug_marker->mutable_color()->set_red(kRed1);
      debug_marker->mutable_color()->set_green(kGreen1);
      debug_marker->mutable_color()->set_blue(kBlue1);
    }

    {
      GpuDebugMarker* debug_marker = gpu_queue_submission->add_completed_markers();
      debug_marker->mutable_begin_marker()->set_gpu_timestamp_ns(kTimestampNs2 + 40);
      debug_marker->mutable_begin_marker()->mutable_meta_info()->set_tid(kTid2 + 1);
      debug_marker->mutable_begin_marker()->mutable_meta_info()->set_pre_submission_cpu_timestamp(
          kTimestampNs2 + 100);
      debug_marker->mutable_begin_marker()->mutable_meta_info()->set_post_submission_cpu_timestamp(
          kTimestampNs2 + 101);
      debug_marker->set_text_key(kKey2);
      debug_marker->set_depth(kDepth2);
      debug_marker->set_end_gpu_timestamp_ns(kTimestampNs2 + 50);

      debug_marker->mutable_color()->set_alpha(kAlpha2);
      debug_marker->mutable_color()->set_red(kRed2);
      debug_marker->mutable_color()->set_green(kGreen2);
      debug_marker->mutable_color()->set_blue(kBlue2);
    }
  }

  ClientCaptureEvent string_event1;
  ClientCaptureEvent string_event2;
  ClientCaptureEvent gpu_submission_event;

  EXPECT_CALL(collector, AddEvent)
      .Times(3)
      .WillOnce(SaveArg<0>(&string_event1))
      .WillOnce(SaveArg<0>(&string_event2))
      .WillOnce(SaveArg<0>(&gpu_submission_event));

  producer_event_processor->ProcessEvent(1, std::move(producer_string_event1));
  producer_event_processor->ProcessEvent(1, std::move(producer_string_event2));
  producer_event_processor->ProcessEvent(1, std::move(producer_gpu_submission_event));

  ASSERT_EQ(string_event1.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(string_event2.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(gpu_submission_event.event_case(), ClientCaptureEvent::kGpuQueueSubmission);

  uint64_t string_key1 = string_event1.interned_string().key();
  uint64_t string_key2 = string_event2.interned_string().key();
  EXPECT_NE(string_key1, orbit_grpc_protos::kInvalidInternId);
  EXPECT_EQ(string_event1.interned_string().intern(), "debug marker1");
  EXPECT_NE(string_key2, orbit_grpc_protos::kInvalidInternId);
  EXPECT_EQ(string_event2.interned_string().intern(), "debug marker2");

  const GpuQueueSubmission& gpu_queue_submission = gpu_submission_event.gpu_queue_submission();
  EXPECT_EQ(gpu_queue_submission.meta_info().tid(), kTid1);
  EXPECT_EQ(gpu_queue_submission.meta_info().pre_submission_cpu_timestamp(), kTimestampNs1);
  EXPECT_EQ(gpu_queue_submission.meta_info().post_submission_cpu_timestamp(), kTimestampNs1 + 1);
  EXPECT_EQ(gpu_queue_submission.num_begin_markers(), 2);
  ASSERT_EQ(gpu_queue_submission.submit_infos_size(), 2);
  {
    const GpuSubmitInfo& submit_info = gpu_queue_submission.submit_infos(0);
    ASSERT_EQ(submit_info.command_buffers_size(), 2);
    EXPECT_EQ(submit_info.command_buffers(0).begin_gpu_timestamp_ns(), kTimestampNs1 + 10);
    EXPECT_EQ(submit_info.command_buffers(0).end_gpu_timestamp_ns(), kTimestampNs1 + 11);
    EXPECT_EQ(submit_info.command_buffers(1).begin_gpu_timestamp_ns(), kTimestampNs1 + 20);
    EXPECT_EQ(submit_info.command_buffers(1).end_gpu_timestamp_ns(), kTimestampNs1 + 21);
  }
  {
    const GpuSubmitInfo& submit_info = gpu_queue_submission.submit_infos(1);
    ASSERT_EQ(submit_info.command_buffers_size(), 1);
    EXPECT_EQ(submit_info.command_buffers(0).begin_gpu_timestamp_ns(), kTimestampNs1 + 30);
    EXPECT_EQ(submit_info.command_buffers(0).end_gpu_timestamp_ns(), kTimestampNs1 + 31);
  }

  ASSERT_EQ(gpu_queue_submission.completed_markers_size(), 2);
  {
    const GpuDebugMarker& debug_marker = gpu_queue_submission.completed_markers(0);
    EXPECT_EQ(debug_marker.begin_marker().gpu_timestamp_ns(), kTimestampNs1 + 40);
    EXPECT_EQ(debug_marker.begin_marker().meta_info().tid(), kTid1 + 1);
    EXPECT_EQ(debug_marker.begin_marker().meta_info().pre_submission_cpu_timestamp(),
              kTimestampNs1 + 100);
    EXPECT_EQ(debug_marker.begin_marker().meta_info().post_submission_cpu_timestamp(),
              kTimestampNs1 + 101);
    EXPECT_EQ(debug_marker.text_key(), string_key1);
    EXPECT_EQ(debug_marker.depth(), kDepth1);
    EXPECT_EQ(debug_marker.end_gpu_timestamp_ns(), kTimestampNs1 + 50);

    EXPECT_EQ(debug_marker.color().alpha(), kAlpha1);
    EXPECT_EQ(debug_marker.color().red(), kRed1);
    EXPECT_EQ(debug_marker.color().green(), kGreen1);
    EXPECT_EQ(debug_marker.color().blue(), kBlue1);
  }

  {
    const GpuDebugMarker& debug_marker = gpu_queue_submission.completed_markers(1);
    EXPECT_EQ(debug_marker.begin_marker().gpu_timestamp_ns(), kTimestampNs2 + 40);
    EXPECT_EQ(debug_marker.begin_marker().meta_info().tid(), kTid2 + 1);
    EXPECT_EQ(debug_marker.begin_marker().meta_info().pre_submission_cpu_timestamp(),
              kTimestampNs2 + 100);
    EXPECT_EQ(debug_marker.begin_marker().meta_info().post_submission_cpu_timestamp(),
              kTimestampNs2 + 101);
    EXPECT_EQ(debug_marker.text_key(), string_key2);
    EXPECT_EQ(debug_marker.depth(), kDepth2);
    EXPECT_EQ(debug_marker.end_gpu_timestamp_ns(), kTimestampNs2 + 50);

    EXPECT_EQ(debug_marker.color().alpha(), kAlpha2);
    EXPECT_EQ(debug_marker.color().red(), kRed2);
    EXPECT_EQ(debug_marker.color().green(), kGreen2);
    EXPECT_EQ(debug_marker.color().blue(), kBlue2);
  }
}

TEST(ProducerEventProcessor, ThreadNameSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_thread_name;
  producer_thread_name.mutable_thread_name()->set_pid(kPid1);
  producer_thread_name.mutable_thread_name()->set_tid(kTid1);
  producer_thread_name.mutable_thread_name()->set_timestamp_ns(kTimestampNs1);
  producer_thread_name.mutable_thread_name()->set_name("Main Thread");

  ClientCaptureEvent event;

  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&event));

  producer_event_processor->ProcessEvent(1, std::move(producer_thread_name));

  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kThreadName);
  EXPECT_EQ(event.thread_name().pid(), kPid1);
  EXPECT_EQ(event.thread_name().tid(), kTid1);
  EXPECT_EQ(event.thread_name().timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(event.thread_name().name(), "Main Thread");
}

::testing::Matcher<ThreadStateSlice> ThreadStateSliceEq(const ThreadStateSlice& expected) {
  return ::testing::AllOf(
      ::testing::Property("tid", &ThreadStateSlice::tid, expected.tid()),
      ::testing::Property("thread_state", &ThreadStateSlice::thread_state, expected.thread_state()),
      ::testing::Property("duration_ns", &ThreadStateSlice::duration_ns, expected.duration_ns()),
      ::testing::Property("end_timestamp_ns", &ThreadStateSlice::end_timestamp_ns,
                          expected.end_timestamp_ns()),
      ::testing::Property("wakeup_reason", &ThreadStateSlice::wakeup_reason,
                          expected.wakeup_reason()),
      ::testing::Property("wakeup_tid", &ThreadStateSlice::wakeup_tid, expected.wakeup_tid()),
      ::testing::Property("wakeup_pid", &ThreadStateSlice::wakeup_pid, expected.wakeup_pid()),
      ::testing::Property("switch_out_or_wakeup_callstack_status",
                          &ThreadStateSlice::switch_out_or_wakeup_callstack_status,
                          expected.switch_out_or_wakeup_callstack_status()),
      ::testing::Property("switch_out_or_wakeup_callstack_id",
                          &ThreadStateSlice::switch_out_or_wakeup_callstack_id,
                          expected.switch_out_or_wakeup_callstack_id()));
}

::testing::Matcher<Callstack> CallstackEq(const Callstack& expected) {
  return ::testing::AllOf(
      ::testing::Property("pcs", &Callstack::pcs, ElementsAreArray(expected.pcs())),
      ::testing::Property("type", &Callstack::type, expected.type()));
}

::testing::Matcher<InternedCallstack> InternedCallstackEq(const InternedCallstack& expected) {
  return ::testing::AllOf(
      ::testing::Property("key", &InternedCallstack::key, expected.key()),
      ::testing::Property("intern", &InternedCallstack::intern, CallstackEq(expected.intern())));
}

::testing::Matcher<ClientCaptureEvent> ClientCaptureEventsTheadStateSliceEq(
    const ThreadStateSlice& expected_thread_state_slice) {
  return ::testing::AllOf(
      ::testing::Property("has_thread_state_slice", &ClientCaptureEvent::has_thread_state_slice,
                          ::testing::IsTrue()),
      ::testing::Property("thread_state_slice", &ClientCaptureEvent::thread_state_slice,
                          ThreadStateSliceEq(expected_thread_state_slice)));
}

::testing::Matcher<ClientCaptureEvent> ClientCaptureEventsInternedCallstackEq(
    const InternedCallstack& expected_interned_callstack) {
  return ::testing::AllOf(
      ::testing::Property("has_interned_callstack", &ClientCaptureEvent::has_interned_callstack,
                          ::testing::IsTrue()),
      ::testing::Property("interned_callstack", &ClientCaptureEvent::interned_callstack,
                          InternedCallstackEq(expected_interned_callstack)));
}

TEST(ProducerEventProcessor, ThreadStateSliceSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_event;
  ThreadStateSlice* producer_thread_state_slice = producer_event.mutable_thread_state_slice();
  producer_thread_state_slice->set_pid(kPid1);
  producer_thread_state_slice->set_tid(kTid1);
  producer_thread_state_slice->set_thread_state(ThreadStateSlice::kIdle);
  producer_thread_state_slice->set_duration_ns(kDurationNs1);
  producer_thread_state_slice->set_end_timestamp_ns(kTimestampNs1);
  producer_thread_state_slice->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kNoCallstack);
  producer_thread_state_slice->set_switch_out_or_wakeup_callstack_id(0);

  ThreadStateSlice expected_thread_state_slice = *producer_thread_state_slice;

  ClientCaptureEvent event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&event));

  producer_event_processor->ProcessEvent(orbit_grpc_protos::kLinuxTracingProducerId,
                                         std::move(producer_event));

  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kThreadStateSlice);
  EXPECT_THAT(event.thread_state_slice(), ThreadStateSliceEq(expected_thread_state_slice));
}

TEST(ProducerEventProcessor, MergingThreadStateSliceWithCallstack) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  constexpr uint64_t kCallstackFrame1 = 1;
  constexpr uint64_t kCallstackFrame2 = 2;
  constexpr uint64_t kCallstackFrame3 = 3;

  ProducerCaptureEvent thread_state_slice_callstack_event1;
  ThreadStateSliceCallstack* thread_state_slice_callstack1 =
      thread_state_slice_callstack_event1.mutable_thread_state_slice_callstack();
  thread_state_slice_callstack1->set_thread_state_slice_tid(kTid1);
  thread_state_slice_callstack1->set_timestamp_ns(kTimestampNs1 - kDurationNs1);
  Callstack* callstack1 = thread_state_slice_callstack1->mutable_callstack();
  callstack1->add_pcs(kCallstackFrame1);
  callstack1->add_pcs(kCallstackFrame2);
  callstack1->add_pcs(kCallstackFrame3);

  ProducerCaptureEvent thread_state_slice_event1;
  ThreadStateSlice* thread_state_slice1 = thread_state_slice_event1.mutable_thread_state_slice();
  thread_state_slice1->set_pid(kPid1);
  thread_state_slice1->set_tid(kTid1);
  thread_state_slice1->set_thread_state(ThreadStateSlice::kRunnable);
  thread_state_slice1->set_duration_ns(kDurationNs1);
  thread_state_slice1->set_end_timestamp_ns(kTimestampNs1);
  thread_state_slice1->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kWaitingForCallstack);
  thread_state_slice1->set_switch_out_or_wakeup_callstack_id(0);
  ThreadStateSlice expected_thread_state_slice1 = *thread_state_slice1;

  ProducerCaptureEvent thread_state_slice_callstack_event2;
  ThreadStateSliceCallstack* thread_state_slice_callstack2 =
      thread_state_slice_callstack_event2.mutable_thread_state_slice_callstack();
  thread_state_slice_callstack2->set_thread_state_slice_tid(kTid2);
  thread_state_slice_callstack2->set_timestamp_ns(kTimestampNs2 - kDurationNs2);
  Callstack* callstack2 = thread_state_slice_callstack2->mutable_callstack();
  callstack2->add_pcs(kCallstackFrame1);
  callstack2->add_pcs(kCallstackFrame2);
  callstack2->add_pcs(kCallstackFrame3);

  ProducerCaptureEvent thread_state_slice_event2;
  ThreadStateSlice* thread_state_slice2 = thread_state_slice_event2.mutable_thread_state_slice();
  thread_state_slice2->set_pid(kPid1);
  thread_state_slice2->set_tid(kTid2);
  thread_state_slice2->set_thread_state(ThreadStateSlice::kRunnable);
  thread_state_slice2->set_duration_ns(kDurationNs2);
  thread_state_slice2->set_end_timestamp_ns(kTimestampNs2);
  thread_state_slice2->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kWaitingForCallstack);
  thread_state_slice2->set_switch_out_or_wakeup_callstack_id(0);
  ThreadStateSlice expected_thread_state_slice2 = *thread_state_slice2;

  std::vector<ClientCaptureEvent> actual_client_capture_events;

  std::optional<uint64_t> actual_callstack_key;
  EXPECT_CALL(collector, AddEvent)
      .Times(3)
      .WillRepeatedly(Invoke([&actual_client_capture_events,
                              &actual_callstack_key](ClientCaptureEvent&& client_capture_event) {
        if (client_capture_event.has_interned_callstack()) {
          // We are only expecting a single interned callstack
          ASSERT_FALSE(actual_callstack_key.has_value());
          actual_callstack_key = client_capture_event.interned_callstack().key();
        }
        actual_client_capture_events.push_back(std::move(client_capture_event));
      }));

  producer_event_processor->ProcessEvent(orbit_grpc_protos::kLinuxTracingProducerId,
                                         std::move(thread_state_slice_callstack_event1));
  producer_event_processor->ProcessEvent(orbit_grpc_protos::kLinuxTracingProducerId,
                                         std::move(thread_state_slice_event1));
  producer_event_processor->ProcessEvent(orbit_grpc_protos::kLinuxTracingProducerId,
                                         std::move(thread_state_slice_callstack_event2));
  producer_event_processor->ProcessEvent(orbit_grpc_protos::kLinuxTracingProducerId,
                                         std::move(thread_state_slice_event2));

  InternedCallstack expected_interned_callstack;
  ASSERT_TRUE(actual_callstack_key.has_value());
  expected_interned_callstack.set_key(
      actual_callstack_key.value());  // We only care that the thread state slices have the same key
  expected_interned_callstack.mutable_intern()->add_pcs(1);
  expected_interned_callstack.mutable_intern()->add_pcs(2);
  expected_interned_callstack.mutable_intern()->add_pcs(3);
  expected_interned_callstack.mutable_intern()->set_type(Callstack::kComplete);

  expected_thread_state_slice1.set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kCallstackSet);
  expected_thread_state_slice1.set_switch_out_or_wakeup_callstack_id(actual_callstack_key.value());
  expected_thread_state_slice2.set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kCallstackSet);
  expected_thread_state_slice2.set_switch_out_or_wakeup_callstack_id(actual_callstack_key.value());

  EXPECT_THAT(actual_client_capture_events,
              ElementsAre(ClientCaptureEventsInternedCallstackEq(expected_interned_callstack),
                          ClientCaptureEventsTheadStateSliceEq(expected_thread_state_slice1),
                          ClientCaptureEventsTheadStateSliceEq(expected_thread_state_slice2)));
}

TEST(ProducerEventProcessor,
     MergingThreadStateSliceWithCallstackFailsGracefullyWhenNoCallstackWasSeen) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent thread_state_slice_event;
  ThreadStateSlice* thread_state_slice = thread_state_slice_event.mutable_thread_state_slice();
  thread_state_slice->set_pid(kPid1);
  thread_state_slice->set_tid(kTid1);
  thread_state_slice->set_thread_state(ThreadStateSlice::kRunnable);
  thread_state_slice->set_duration_ns(kDurationNs1);
  thread_state_slice->set_end_timestamp_ns(kTimestampNs1);
  thread_state_slice->set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kWaitingForCallstack);
  thread_state_slice->set_switch_out_or_wakeup_callstack_id(0);

  ThreadStateSlice expected_thread_state_slice = *thread_state_slice;
  expected_thread_state_slice.set_switch_out_or_wakeup_callstack_status(
      ThreadStateSlice::kNoCallstack);

  ClientCaptureEvent actual_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&actual_event));

  producer_event_processor->ProcessEvent(orbit_grpc_protos::kLinuxTracingProducerId,
                                         std::move(thread_state_slice_event));

  ASSERT_EQ(actual_event.event_case(), ClientCaptureEvent::kThreadStateSlice);
  EXPECT_THAT(actual_event.thread_state_slice(), ThreadStateSliceEq(expected_thread_state_slice));
}

TEST(ProducerEventProcessor, ModuleUpdateEventSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_event;
  {
    ModuleUpdateEvent* module_update_event = producer_event.mutable_module_update_event();
    module_update_event->set_pid(kPid1);
    module_update_event->set_timestamp_ns(kTimestampNs1);
    ModuleInfo* module_info = module_update_event->mutable_module();
    module_info->set_name("module");
    module_info->set_file_path("file path");
    module_info->set_file_size(1000);
    module_info->set_address_start(5000);
    module_info->set_address_end(7000);
    module_info->set_build_id("build id 42");
    module_info->set_load_bias(0x2000);
  }

  ClientCaptureEvent event;

  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&event));

  producer_event_processor->ProcessEvent(1, std::move(producer_event));

  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kModuleUpdateEvent);
  const ModuleUpdateEvent& module_update = event.module_update_event();
  EXPECT_EQ(module_update.pid(), kPid1);
  EXPECT_EQ(module_update.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(module_update.module().name(), "module");
  EXPECT_EQ(module_update.module().file_path(), "file path");
  EXPECT_EQ(module_update.module().file_size(), 1000);
  EXPECT_EQ(module_update.module().address_start(), 5000);
  EXPECT_EQ(module_update.module().address_end(), 7000);
  EXPECT_EQ(module_update.module().build_id(), "build id 42");
  EXPECT_EQ(module_update.module().load_bias(), 0x2000);
}

TEST(ProducerEventProcessor, FullAddressInfoSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_event1;
  {
    FullAddressInfo* address_info = producer_event1.mutable_full_address_info();
    address_info->set_absolute_address(1000);
    address_info->set_offset_in_function(10);
    address_info->set_function_name("function1");
    address_info->set_module_name("module");
  }

  ProducerCaptureEvent producer_event2;
  {
    FullAddressInfo* address_info = producer_event2.mutable_full_address_info();
    address_info->set_absolute_address(2000);
    address_info->set_offset_in_function(20);
    address_info->set_function_name("function2");
    address_info->set_module_name("module");
  }

  ClientCaptureEvent interned_string_function1;
  ClientCaptureEvent interned_string_module;
  ClientCaptureEvent interned_string_function2;
  ClientCaptureEvent address_info_event1;
  ClientCaptureEvent address_info_event2;

  EXPECT_CALL(collector, AddEvent)
      .Times(5)
      .WillOnce(SaveArg<0>(&interned_string_function1))
      .WillOnce(SaveArg<0>(&interned_string_module))
      .WillOnce(SaveArg<0>(&address_info_event1))
      .WillOnce(SaveArg<0>(&interned_string_function2))
      .WillOnce(SaveArg<0>(&address_info_event2));

  producer_event_processor->ProcessEvent(1, std::move(producer_event1));
  producer_event_processor->ProcessEvent(1, std::move(producer_event2));

  ASSERT_EQ(interned_string_function1.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(interned_string_function2.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(interned_string_module.event_case(), ClientCaptureEvent::kInternedString);
  ASSERT_EQ(address_info_event1.event_case(), ClientCaptureEvent::kAddressInfo);
  ASSERT_EQ(address_info_event2.event_case(), ClientCaptureEvent::kAddressInfo);

  EXPECT_EQ(interned_string_function1.interned_string().intern(), "function1");
  EXPECT_EQ(interned_string_function2.interned_string().intern(), "function2");
  EXPECT_EQ(interned_string_module.interned_string().intern(), "module");

  const uint64_t module_key = interned_string_module.interned_string().key();
  const uint64_t function1_key = interned_string_function1.interned_string().key();
  const uint64_t function2_key = interned_string_function2.interned_string().key();

  EXPECT_NE(module_key, orbit_grpc_protos::kInvalidInternId);
  EXPECT_NE(function1_key, orbit_grpc_protos::kInvalidInternId);
  EXPECT_NE(function2_key, orbit_grpc_protos::kInvalidInternId);

  {
    const AddressInfo& address_info = address_info_event1.address_info();
    EXPECT_EQ(address_info.absolute_address(), 1000);
    EXPECT_EQ(address_info.offset_in_function(), 10);
    EXPECT_EQ(address_info.function_name_key(), function1_key);
    EXPECT_EQ(address_info.module_name_key(), module_key);
  }

  {
    const AddressInfo& address_info = address_info_event2.address_info();
    EXPECT_EQ(address_info.absolute_address(), 2000);
    EXPECT_EQ(address_info.offset_in_function(), 20);
    EXPECT_EQ(address_info.function_name_key(), function2_key);
    EXPECT_EQ(address_info.module_name_key(), module_key);
  }
}

TEST(ProducerEventProcessor, TwoInternedStringsSameProducerSameKey) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1 = CreateInternedStringEvent(kKey1, "string1");
  ProducerCaptureEvent event2 = CreateInternedStringEvent(kKey1, "string2");

  producer_event_processor->ProcessEvent(1, std::move(event1));
  EXPECT_DEATH(producer_event_processor->ProcessEvent(1, std::move(event2)), "");
}

TEST(ProducerEventProcessor, TwoInternedCallstacksSameProducerSameKey) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent event1;
  {
    InternedCallstack* callstack = event1.mutable_interned_callstack();
    callstack->set_key(kKey1);
    callstack->mutable_intern()->add_pcs(1);
    callstack->mutable_intern()->set_type(Callstack::kComplete);
  }

  ProducerCaptureEvent event2;
  {
    InternedCallstack* callstack = event2.mutable_interned_callstack();
    callstack->set_key(kKey1);
    callstack->mutable_intern()->add_pcs(2);
    callstack->mutable_intern()->set_type(Callstack::kComplete);
  }

  producer_event_processor->ProcessEvent(1, std::move(event1));
  EXPECT_DEATH(producer_event_processor->ProcessEvent(1, std::move(event2)), "");
}

TEST(ProducerEventProcessor, CaptureStartedSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_event;
  {
    CaptureStarted* capture_started = producer_event.mutable_capture_started();
    capture_started->set_process_id(kPid1);
    capture_started->set_executable_path(kExecutablePath);
    capture_started->set_executable_build_id(kBuildId1);
    capture_started->set_capture_start_timestamp_ns(kTimestampNs1);

    CaptureOptions* capture_options = capture_started->mutable_capture_options();
    capture_options->set_trace_context_switches(true);
    capture_options->set_samples_per_second(5);
    capture_options->set_unwinding_method(CaptureOptions::kFramePointers);
    capture_options->set_trace_thread_state(true);
    capture_options->set_trace_gpu_driver(true);
    capture_options->set_enable_introspection(true);
    capture_options->set_max_local_marker_depth_per_command_buffer(6);
    capture_options->set_collect_memory_info(true);
    capture_options->set_memory_sampling_period_ns(1001);

    InstrumentedFunction* instrumented_function = capture_options->add_instrumented_functions();
    instrumented_function->set_function_name("void foo()");
    instrumented_function->set_function_id(kFunctionId1);
    instrumented_function->set_function_virtual_address(223433);
    instrumented_function->set_file_offset(123433);
    instrumented_function->set_file_path("path");
    instrumented_function->set_file_build_id(kBuildId2);
  }

  ClientCaptureEvent event;

  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&event));

  producer_event_processor->ProcessEvent(1, std::move(producer_event));

  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kCaptureStarted);
  const CaptureStarted& capture_started = event.capture_started();
  EXPECT_EQ(capture_started.process_id(), kPid1);
  EXPECT_EQ(capture_started.executable_path(), kExecutablePath);
  EXPECT_EQ(capture_started.executable_build_id(), kBuildId1);
  EXPECT_EQ(capture_started.capture_start_timestamp_ns(), kTimestampNs1);

  EXPECT_TRUE(capture_started.capture_options().trace_context_switches());
  EXPECT_EQ(capture_started.capture_options().samples_per_second(), 5);
  EXPECT_EQ(capture_started.capture_options().unwinding_method(), CaptureOptions::kFramePointers);
  EXPECT_TRUE(capture_started.capture_options().trace_thread_state());
  EXPECT_TRUE(capture_started.capture_options().trace_gpu_driver());
  EXPECT_TRUE(capture_started.capture_options().enable_introspection());
  EXPECT_EQ(capture_started.capture_options().max_local_marker_depth_per_command_buffer(), 6);
  EXPECT_TRUE(capture_started.capture_options().collect_memory_info());
  EXPECT_EQ(capture_started.capture_options().memory_sampling_period_ns(), 1001);

  ASSERT_EQ(capture_started.capture_options().instrumented_functions_size(), 1);
  const InstrumentedFunction& instrumented_function =
      capture_started.capture_options().instrumented_functions(0);
  EXPECT_EQ(instrumented_function.function_name(), "void foo()");
  EXPECT_EQ(instrumented_function.function_id(), kFunctionId1);
  EXPECT_EQ(instrumented_function.function_virtual_address(), 223433);
  EXPECT_EQ(instrumented_function.file_offset(), 123433);
  EXPECT_EQ(instrumented_function.file_path(), "path");
  EXPECT_EQ(instrumented_function.file_build_id(), kBuildId2);

  EXPECT_EQ(capture_started.capture_options().instrumented_tracepoint_size(), 0);
}

TEST(ProducerEventProcessor, CaptureFinishedSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ClientCaptureEvent client_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_event));

  {
    ProducerCaptureEvent producer_event;

    CaptureFinished* capture_finished = producer_event.mutable_capture_finished();
    capture_finished->set_status(CaptureFinished::kFailed);
    capture_finished->set_error_message("error_message");

    producer_event_processor->ProcessEvent(1, std::move(producer_event));
  }

  ASSERT_EQ(client_event.event_case(), ClientCaptureEvent::kCaptureFinished);
  const CaptureFinished& capture_finished = client_event.capture_finished();
  EXPECT_EQ(capture_finished.status(), CaptureFinished::kFailed);
  EXPECT_EQ(capture_finished.error_message(), "error_message");
}

TEST(ProducerEventProcessor, ModulesSnapshotSmoke) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_event;
  {
    ModulesSnapshot* modules_snapshot = producer_event.mutable_modules_snapshot();
    modules_snapshot->set_pid(kPid1);
    modules_snapshot->set_timestamp_ns(kTimestampNs1);

    ModuleInfo* module_info = modules_snapshot->add_modules();
    module_info->set_name("module");
    module_info->set_file_path("file path");
    module_info->set_file_size(1000);
    module_info->set_address_start(5000);
    module_info->set_address_end(7000);
    module_info->set_build_id("build id 42");
    module_info->set_load_bias(0x2000);
  }

  ClientCaptureEvent event;

  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&event));

  producer_event_processor->ProcessEvent(1, std::move(producer_event));

  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kModulesSnapshot);
  const ModulesSnapshot& modules_snapshot = event.modules_snapshot();
  EXPECT_EQ(modules_snapshot.pid(), kPid1);
  EXPECT_EQ(modules_snapshot.timestamp_ns(), kTimestampNs1);
  ASSERT_EQ(modules_snapshot.modules_size(), 1);
  EXPECT_EQ(modules_snapshot.modules(0).name(), "module");
  EXPECT_EQ(modules_snapshot.modules(0).file_path(), "file path");
  EXPECT_EQ(modules_snapshot.modules(0).file_size(), 1000);
  EXPECT_EQ(modules_snapshot.modules(0).address_start(), 5000);
  EXPECT_EQ(modules_snapshot.modules(0).address_end(), 7000);
  EXPECT_EQ(modules_snapshot.modules(0).build_id(), "build id 42");
  EXPECT_EQ(modules_snapshot.modules(0).load_bias(), 0x2000);
}

TEST(ProducerEventProcessor, ThreadNamesSnapshot) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_event;
  {
    ThreadNamesSnapshot* thread_names_snapshot = producer_event.mutable_thread_names_snapshot();
    thread_names_snapshot->set_timestamp_ns(kTimestampNs1);
    ThreadName* thread_name = thread_names_snapshot->add_thread_names();
    thread_name->set_pid(kPid1);
    thread_name->set_tid(kTid1);
    thread_name->set_name("Main Thread");
    thread_name->set_timestamp_ns(kTimestampNs2);
  }

  ClientCaptureEvent event;

  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&event));

  producer_event_processor->ProcessEvent(1, std::move(producer_event));

  ASSERT_EQ(event.event_case(), ClientCaptureEvent::kThreadNamesSnapshot);

  const ThreadNamesSnapshot& thread_names_snapshot = event.thread_names_snapshot();
  EXPECT_EQ(thread_names_snapshot.timestamp_ns(), kTimestampNs1);
  ASSERT_EQ(thread_names_snapshot.thread_names_size(), 1);
  const ThreadName& thread_name = thread_names_snapshot.thread_names(0);
  EXPECT_EQ(thread_name.pid(), kPid1);
  EXPECT_EQ(thread_name.tid(), kTid1);
  EXPECT_EQ(thread_name.name(), "Main Thread");
  EXPECT_EQ(thread_name.timestamp_ns(), kTimestampNs2);
}

TEST(ProducerEventProcessor, MemoryUsageEvent) {
  ProducerCaptureEvent producer_capture_event;
  MemoryUsageEvent* memory_usage_event = producer_capture_event.mutable_memory_usage_event();
  memory_usage_event->set_timestamp_ns(kTimestampNs1);

  SystemMemoryUsage* system_memory_usage = memory_usage_event->mutable_system_memory_usage();
  system_memory_usage->set_timestamp_ns(kTimestampNs2);
  system_memory_usage->set_total_kb(10);
  system_memory_usage->set_free_kb(20);
  system_memory_usage->set_available_kb(30);
  system_memory_usage->set_buffers_kb(40);
  system_memory_usage->set_cached_kb(50);
  system_memory_usage->set_pgfault(60);
  system_memory_usage->set_pgmajfault(70);

  CGroupMemoryUsage* cgroup_memory_usage = memory_usage_event->mutable_cgroup_memory_usage();
  cgroup_memory_usage->set_timestamp_ns(kTimestampNs2);
  cgroup_memory_usage->set_cgroup_name("memory_cgroup_name");
  cgroup_memory_usage->set_limit_bytes(10);
  cgroup_memory_usage->set_rss_bytes(20);
  cgroup_memory_usage->set_mapped_file_bytes(30);
  cgroup_memory_usage->set_pgfault(40);
  cgroup_memory_usage->set_pgmajfault(50);
  cgroup_memory_usage->set_unevictable_bytes(60);
  cgroup_memory_usage->set_inactive_anon_bytes(70);
  cgroup_memory_usage->set_active_anon_bytes(80);
  cgroup_memory_usage->set_inactive_file_bytes(90);
  cgroup_memory_usage->set_active_file_bytes(100);

  ProcessMemoryUsage* process_memory_usage = memory_usage_event->mutable_process_memory_usage();
  process_memory_usage->set_timestamp_ns(kTimestampNs2);
  process_memory_usage->set_pid(1234);
  process_memory_usage->set_rss_anon_kb(10);
  process_memory_usage->set_minflt(20);
  process_memory_usage->set_minflt(30);

  std::string expected_memory_usage_event_serialized_as_string =
      memory_usage_event->SerializeAsString();

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kMemoryInfoProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kMemoryUsageEvent);
  const MemoryUsageEvent& actual_memory_usage_event = client_capture_event.memory_usage_event();
  ASSERT_EQ(actual_memory_usage_event.SerializeAsString(),
            expected_memory_usage_event_serialized_as_string);
}

TEST(ProducerEventProcessor, ApiScopeStart) {
  ProducerCaptureEvent producer_capture_event;
  ApiScopeStart* api_scope_start = producer_capture_event.mutable_api_scope_start();
  api_scope_start->set_pid(kPid1);
  api_scope_start->set_tid(kTid1);
  api_scope_start->set_timestamp_ns(kTimestampNs1);
  api_scope_start->set_color_rgba(kColor1);
  api_scope_start->set_group_id(kGroupId1);
  api_scope_start->set_address_in_function(kFunctionId1);
  api_scope_start->set_encoded_name_1(kEncodedName1);
  api_scope_start->set_encoded_name_2(kEncodedName2);
  api_scope_start->set_encoded_name_3(kEncodedName3);
  api_scope_start->set_encoded_name_4(kEncodedName4);
  api_scope_start->set_encoded_name_5(kEncodedName5);
  api_scope_start->set_encoded_name_6(kEncodedName6);
  api_scope_start->set_encoded_name_7(kEncodedName7);
  api_scope_start->set_encoded_name_8(kEncodedName8);
  api_scope_start->add_encoded_name_additional(kEncodedNameAdditional1);

  ApiScopeStart api_scope_start_copy = *api_scope_start;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiScopeStart);
  const ApiScopeStart& actual_event = client_capture_event.api_scope_start();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_scope_start_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiScopeStop) {
  ProducerCaptureEvent producer_capture_event;
  ApiScopeStop* api_scope_stop = producer_capture_event.mutable_api_scope_stop();
  api_scope_stop->set_pid(kPid1);
  api_scope_stop->set_tid(kTid1);
  api_scope_stop->set_timestamp_ns(kTimestampNs1);
  ApiScopeStop api_scope_stop_copy = *api_scope_stop;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiScopeStop);
  const ApiScopeStop& actual_event = client_capture_event.api_scope_stop();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_scope_stop_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiScopeStartAsync) {
  ProducerCaptureEvent producer_capture_event;
  ApiScopeStartAsync* api_scope_start_async =
      producer_capture_event.mutable_api_scope_start_async();
  api_scope_start_async->set_pid(kPid1);
  api_scope_start_async->set_tid(kTid1);
  api_scope_start_async->set_timestamp_ns(kTimestampNs1);
  api_scope_start_async->set_color_rgba(kColor1);
  api_scope_start_async->set_id(kGroupId1);
  api_scope_start_async->set_address_in_function(kFunctionId1);
  api_scope_start_async->set_encoded_name_1(kEncodedName1);
  api_scope_start_async->set_encoded_name_2(kEncodedName2);
  api_scope_start_async->set_encoded_name_3(kEncodedName3);
  api_scope_start_async->set_encoded_name_4(kEncodedName4);
  api_scope_start_async->set_encoded_name_5(kEncodedName5);
  api_scope_start_async->set_encoded_name_6(kEncodedName6);
  api_scope_start_async->set_encoded_name_7(kEncodedName7);
  api_scope_start_async->set_encoded_name_8(kEncodedName8);
  api_scope_start_async->add_encoded_name_additional(kEncodedNameAdditional1);
  ApiScopeStartAsync api_scope_start_async_copy = *api_scope_start_async;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiScopeStartAsync);
  const ApiScopeStartAsync& actual_event = client_capture_event.api_scope_start_async();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_scope_start_async_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiScopeStopAsync) {
  ProducerCaptureEvent producer_capture_event;
  ApiScopeStopAsync* api_scope_stop_async = producer_capture_event.mutable_api_scope_stop_async();
  api_scope_stop_async->set_pid(kPid1);
  api_scope_stop_async->set_tid(kTid1);
  api_scope_stop_async->set_timestamp_ns(kTimestampNs1);
  api_scope_stop_async->set_id(kGroupId1);
  ApiScopeStopAsync api_scope_stop_async_copy = *api_scope_stop_async;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiScopeStopAsync);
  const ApiScopeStopAsync& actual_event = client_capture_event.api_scope_stop_async();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_scope_stop_async_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiStringEvent) {
  ProducerCaptureEvent producer_capture_event;
  ApiStringEvent* api_string_event = producer_capture_event.mutable_api_string_event();
  api_string_event->set_pid(kPid1);
  api_string_event->set_tid(kTid1);
  api_string_event->set_timestamp_ns(kTimestampNs1);
  api_string_event->set_id(kGroupId1);
  api_string_event->set_encoded_name_1(kEncodedName1);
  api_string_event->set_encoded_name_2(kEncodedName2);
  api_string_event->set_encoded_name_3(kEncodedName3);
  api_string_event->set_encoded_name_4(kEncodedName4);
  api_string_event->set_encoded_name_5(kEncodedName5);
  api_string_event->set_encoded_name_6(kEncodedName6);
  api_string_event->set_encoded_name_7(kEncodedName7);
  api_string_event->set_encoded_name_8(kEncodedName8);
  api_string_event->add_encoded_name_additional(kEncodedNameAdditional1);
  ApiStringEvent api_string_event_copy = *api_string_event;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiStringEvent);
  const ApiStringEvent& actual_event = client_capture_event.api_string_event();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_string_event_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiTrackInt) {
  ProducerCaptureEvent producer_capture_event;
  ApiTrackInt* api_track_int = producer_capture_event.mutable_api_track_int();
  api_track_int->set_pid(kPid1);
  api_track_int->set_tid(kTid1);
  api_track_int->set_timestamp_ns(kTimestampNs1);
  api_track_int->set_data(kInt);
  ApiTrackInt api_track_int_copy = *api_track_int;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiTrackInt);
  const ApiTrackInt& actual_event = client_capture_event.api_track_int();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_track_int_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiTrackInt64) {
  ProducerCaptureEvent producer_capture_event;
  ApiTrackInt64* api_track_int64 = producer_capture_event.mutable_api_track_int64();
  api_track_int64->set_pid(kPid1);
  api_track_int64->set_tid(kTid1);
  api_track_int64->set_timestamp_ns(kTimestampNs1);
  api_track_int64->set_data(kInt64);
  ApiTrackInt64 api_track_int64_copy = *api_track_int64;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiTrackInt64);
  const ApiTrackInt64& actual_event = client_capture_event.api_track_int64();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_track_int64_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiTrackUint) {
  ProducerCaptureEvent producer_capture_event;
  ApiTrackUint* api_track_uint = producer_capture_event.mutable_api_track_uint();
  api_track_uint->set_pid(kPid1);
  api_track_uint->set_tid(kTid1);
  api_track_uint->set_timestamp_ns(kTimestampNs1);
  api_track_uint->set_data(kUint);
  ApiTrackUint api_track_uint_copy = *api_track_uint;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiTrackUint);
  const ApiTrackUint& actual_event = client_capture_event.api_track_uint();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_track_uint_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiTrackUint64) {
  ProducerCaptureEvent producer_capture_event;
  ApiTrackUint64* api_track_uint64 = producer_capture_event.mutable_api_track_uint64();
  api_track_uint64->set_pid(kPid1);
  api_track_uint64->set_tid(kTid1);
  api_track_uint64->set_timestamp_ns(kTimestampNs1);
  api_track_uint64->set_data(kUint64);
  ApiTrackUint64 api_track_uint64_copy = *api_track_uint64;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiTrackUint64);
  const ApiTrackUint64& actual_event = client_capture_event.api_track_uint64();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_track_uint64_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiTrackFloat) {
  ProducerCaptureEvent producer_capture_event;
  ApiTrackFloat* api_track_float = producer_capture_event.mutable_api_track_float();
  api_track_float->set_pid(kPid1);
  api_track_float->set_tid(kTid1);
  api_track_float->set_timestamp_ns(kTimestampNs1);
  api_track_float->set_data(kFloat);
  ApiTrackFloat api_track_float_copy = *api_track_float;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiTrackFloat);
  const ApiTrackFloat& actual_event = client_capture_event.api_track_float();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_track_float_copy, actual_event));
}

TEST(ProducerEventProcessor, ApiTrackDouble) {
  ProducerCaptureEvent producer_capture_event;
  ApiTrackDouble* api_track_double = producer_capture_event.mutable_api_track_double();
  api_track_double->set_pid(kPid1);
  api_track_double->set_tid(kTid1);
  api_track_double->set_timestamp_ns(kTimestampNs1);
  api_track_double->set_data(kDouble);
  ApiTrackDouble api_track_double_copy = *api_track_double;

  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);
  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));
  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kApiTrackDouble);
  const ApiTrackDouble& actual_event = client_capture_event.api_track_double();
  EXPECT_TRUE(MessageDifferencer::Equivalent(api_track_double_copy, actual_event));
}

TEST(ProducerEventProcessor, WarningEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_capture_event;
  WarningEvent* warning_event = producer_capture_event.mutable_warning_event();
  warning_event->set_timestamp_ns(kTimestampNs1);
  constexpr const char* kMessage = "message";
  warning_event->set_message(kMessage);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kWarningEvent);
  const WarningEvent& actual_warning_event = client_capture_event.warning_event();
  EXPECT_EQ(actual_warning_event.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(actual_warning_event.message(), kMessage);
}

TEST(ProducerEventProcessor, ClockResolutionEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_capture_event;
  ClockResolutionEvent* clock_resolution_event =
      producer_capture_event.mutable_clock_resolution_event();
  clock_resolution_event->set_timestamp_ns(kTimestampNs1);
  constexpr uint64_t kClockResolutionNs = 123;
  clock_resolution_event->set_clock_resolution_ns(kClockResolutionNs);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kClockResolutionEvent);
  const ClockResolutionEvent& actual_clock_resolution_event =
      client_capture_event.clock_resolution_event();
  EXPECT_EQ(actual_clock_resolution_event.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(actual_clock_resolution_event.clock_resolution_ns(), kClockResolutionNs);
}

TEST(ProducerEventProcessor, ErrorsWithPerfEventOpenEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_capture_event;
  ErrorsWithPerfEventOpenEvent* errors_with_perf_event_open_event =
      producer_capture_event.mutable_errors_with_perf_event_open_event();
  errors_with_perf_event_open_event->set_timestamp_ns(kTimestampNs1);
  constexpr const char* kFailedToOpen1 = "sampling";
  constexpr const char* kFailedToOpen2 = "uprobes";
  errors_with_perf_event_open_event->add_failed_to_open(kFailedToOpen1);
  errors_with_perf_event_open_event->add_failed_to_open(kFailedToOpen2);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kErrorsWithPerfEventOpenEvent);
  const ErrorsWithPerfEventOpenEvent& actual_errors_with_perf_event_open_event =
      client_capture_event.errors_with_perf_event_open_event();
  EXPECT_EQ(actual_errors_with_perf_event_open_event.timestamp_ns(), kTimestampNs1);
  EXPECT_THAT(std::vector(actual_errors_with_perf_event_open_event.failed_to_open().begin(),
                          actual_errors_with_perf_event_open_event.failed_to_open().end()),
              testing::ElementsAre(kFailedToOpen1, kFailedToOpen2));
}

TEST(ProducerEventProcessor, ErrorEnablingOrbitApiEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_capture_event;
  ErrorEnablingOrbitApiEvent* error_enabling_orbit_api_event =
      producer_capture_event.mutable_error_enabling_orbit_api_event();
  error_enabling_orbit_api_event->set_timestamp_ns(kTimestampNs1);
  constexpr const char* kMessage = "message";
  error_enabling_orbit_api_event->set_message(kMessage);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kErrorEnablingOrbitApiEvent);
  const ErrorEnablingOrbitApiEvent& actual_error_enabling_orbit_api_event =
      client_capture_event.error_enabling_orbit_api_event();
  EXPECT_EQ(actual_error_enabling_orbit_api_event.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(actual_error_enabling_orbit_api_event.message(), kMessage);
}

TEST(ProducerEventProcessor, ErrorEnablingUserSpaceInstrumentationEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_capture_event;
  ErrorEnablingUserSpaceInstrumentationEvent* error_event =
      producer_capture_event.mutable_error_enabling_user_space_instrumentation_event();
  error_event->set_timestamp_ns(kTimestampNs1);
  constexpr const char* kMessage = "message";
  error_event->set_message(kMessage);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));

  ASSERT_EQ(client_capture_event.event_case(),
            ClientCaptureEvent::kErrorEnablingUserSpaceInstrumentationEvent);
  const ErrorEnablingUserSpaceInstrumentationEvent& actual_error_event =
      client_capture_event.error_enabling_user_space_instrumentation_event();
  EXPECT_EQ(actual_error_event.timestamp_ns(), kTimestampNs1);
  EXPECT_EQ(actual_error_event.message(), kMessage);
}

TEST(ProducerEventProcessor, WarningInstrumentingWithUserSpaceInstrumentationEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_capture_event;
  WarningInstrumentingWithUserSpaceInstrumentationEvent* warning_event =
      producer_capture_event.mutable_warning_instrumenting_with_user_space_instrumentation_event();
  warning_event->set_timestamp_ns(kTimestampNs1);
  constexpr int kFunctionId = 42;
  constexpr const char* kErrorMessage = "error message";
  auto* function = warning_event->add_functions_that_failed_to_instrument();
  function->set_function_id(kFunctionId);
  function->set_error_message(kErrorMessage);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));

  ASSERT_EQ(client_capture_event.event_case(),
            ClientCaptureEvent::kWarningInstrumentingWithUserSpaceInstrumentationEvent);
  const WarningInstrumentingWithUserSpaceInstrumentationEvent& actual_warning_event =
      client_capture_event.warning_instrumenting_with_user_space_instrumentation_event();
  EXPECT_EQ(actual_warning_event.timestamp_ns(), kTimestampNs1);
  ASSERT_EQ(actual_warning_event.functions_that_failed_to_instrument_size(), 1);
  EXPECT_EQ(actual_warning_event.functions_that_failed_to_instrument(0).function_id(), kFunctionId);
  EXPECT_EQ(actual_warning_event.functions_that_failed_to_instrument(0).error_message(),
            kErrorMessage);
}

TEST(ProducerEventProcessor, LostPerfRecordsEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_capture_event;
  LostPerfRecordsEvent* lost_perf_records_event =
      producer_capture_event.mutable_lost_perf_records_event();
  lost_perf_records_event->set_duration_ns(kDurationNs1);
  lost_perf_records_event->set_end_timestamp_ns(kTimestampNs1);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kLostPerfRecordsEvent);
  const LostPerfRecordsEvent& actual_lost_perf_records_event =
      client_capture_event.lost_perf_records_event();
  EXPECT_EQ(actual_lost_perf_records_event.duration_ns(), kDurationNs1);
  EXPECT_EQ(actual_lost_perf_records_event.end_timestamp_ns(), kTimestampNs1);
}

TEST(ProducerEventProcessor, OutOfOrderEventsDiscardedEvent) {
  MockClientCaptureEventCollector collector;
  auto producer_event_processor = ProducerEventProcessor::Create(&collector);

  ProducerCaptureEvent producer_capture_event;
  OutOfOrderEventsDiscardedEvent* out_of_order_events_discarded_event =
      producer_capture_event.mutable_out_of_order_events_discarded_event();
  out_of_order_events_discarded_event->set_duration_ns(kDurationNs1);
  out_of_order_events_discarded_event->set_end_timestamp_ns(kTimestampNs1);

  ClientCaptureEvent client_capture_event;
  EXPECT_CALL(collector, AddEvent).Times(1).WillOnce(SaveArg<0>(&client_capture_event));

  producer_event_processor->ProcessEvent(kDefaultProducerId, std::move(producer_capture_event));

  ASSERT_EQ(client_capture_event.event_case(), ClientCaptureEvent::kOutOfOrderEventsDiscardedEvent);
  const OutOfOrderEventsDiscardedEvent& actual_out_of_order_events_discarded_event =
      client_capture_event.out_of_order_events_discarded_event();
  EXPECT_EQ(actual_out_of_order_events_discarded_event.duration_ns(), kDurationNs1);
  EXPECT_EQ(actual_out_of_order_events_discarded_event.end_timestamp_ns(), kTimestampNs1);
}

}  // namespace orbit_producer_event_processor