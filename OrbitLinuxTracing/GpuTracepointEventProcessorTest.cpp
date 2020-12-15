// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "GpuTracepointEventProcessor.h"
#include "OrbitBase/Logging.h"
#include "OrbitLinuxTracing/TracerListener.h"
#include "capture.pb.h"

namespace LinuxTracing {

namespace {

class MockTracerListener : public TracerListener {
 public:
  MOCK_METHOD(void, OnSchedulingSlice, (orbit_grpc_protos::SchedulingSlice), (override));
  MOCK_METHOD(void, OnCallstackSample, (orbit_grpc_protos::CallstackSample), (override));
  MOCK_METHOD(void, OnFunctionCall, (orbit_grpc_protos::FunctionCall), (override));
  MOCK_METHOD(void, OnIntrospectionScope, (orbit_grpc_protos::IntrospectionScope), (override));
  MOCK_METHOD(void, OnGpuJob, (orbit_grpc_protos::GpuJob gpu_job), (override));
  MOCK_METHOD(void, OnThreadName, (orbit_grpc_protos::ThreadName), (override));
  MOCK_METHOD(void, OnThreadStateSlice, (orbit_grpc_protos::ThreadStateSlice), (override));
  MOCK_METHOD(void, OnAddressInfo, (orbit_grpc_protos::AddressInfo), (override));
  MOCK_METHOD(void, OnTracepointEvent, (orbit_grpc_protos::TracepointEvent), (override));
  MOCK_METHOD(void, OnModulesUpdate, (orbit_grpc_protos::ModulesUpdateEvent), (override));
};

class GpuTracepointEventProcessorTest : public ::testing::Test {
 protected:
  void SetUp() override { processor_.SetListener(&mock_listener_); }

  void TearDown() override {}

  GpuTracepointEventProcessor processor_;
  MockTracerListener mock_listener_;
};

AmdgpuCsIoctlPerfEvent MakeFakeAmdgpuCsIoctlPerfEvent(pid_t tid, uint64_t timestamp_ns,
                                                      uint32_t context, uint32_t seqno,
                                                      const std::string& timeline) {
  AmdgpuCsIoctlPerfEvent event{
      static_cast<uint32_t>(sizeof(amdgpu_cs_ioctl_tracepoint) + timeline.length() + 1)};
  event.ring_buffer_record.sample_id.tid = tid;
  CHECK(event.GetTid() == tid);
  event.ring_buffer_record.sample_id.time = timestamp_ns;
  CHECK(event.GetTimestamp() == timestamp_ns);
  reinterpret_cast<amdgpu_cs_ioctl_tracepoint*>(event.tracepoint_data.get())->context = context;
  CHECK(event.GetContext() == context);
  reinterpret_cast<amdgpu_cs_ioctl_tracepoint*>(event.tracepoint_data.get())->seqno = seqno;
  CHECK(event.GetSeqno() == seqno);
  // This logic is the reverse of GpuPerfEvent::ExtractTimelineString.
  reinterpret_cast<amdgpu_cs_ioctl_tracepoint*>(event.tracepoint_data.get())->timeline =
      ((timeline.length() + 1) << 16) | sizeof(amdgpu_cs_ioctl_tracepoint);
  memcpy(event.tracepoint_data.get() + sizeof(amdgpu_cs_ioctl_tracepoint), timeline.c_str(),
         timeline.length() + 1);
  CHECK(event.ExtractTimelineString() == timeline);
  return event;
}

AmdgpuSchedRunJobPerfEvent MakeFakeAmdgpuSchedRunJobPerfEvent(uint64_t timestamp_ns,
                                                              uint32_t context, uint32_t seqno,
                                                              const std::string& timeline) {
  AmdgpuSchedRunJobPerfEvent event{
      static_cast<uint32_t>(sizeof(amdgpu_sched_run_job_tracepoint) + timeline.length() + 1)};
  event.ring_buffer_record.sample_id.time = timestamp_ns;
  CHECK(event.GetTimestamp() == timestamp_ns);
  reinterpret_cast<amdgpu_sched_run_job_tracepoint*>(event.tracepoint_data.get())->context =
      context;
  CHECK(event.GetContext() == context);
  reinterpret_cast<amdgpu_sched_run_job_tracepoint*>(event.tracepoint_data.get())->seqno = seqno;
  CHECK(event.GetSeqno() == seqno);
  reinterpret_cast<amdgpu_sched_run_job_tracepoint*>(event.tracepoint_data.get())->timeline =
      ((timeline.length() + 1) << 16) | sizeof(amdgpu_sched_run_job_tracepoint);
  memcpy(event.tracepoint_data.get() + sizeof(amdgpu_sched_run_job_tracepoint), timeline.c_str(),
         timeline.length() + 1);
  CHECK(event.ExtractTimelineString() == timeline);
  return event;
}

DmaFenceSignaledPerfEvent MakeFakeDmaFenceSignaledPerfEvent(uint64_t timestamp_ns, uint32_t context,
                                                            uint32_t seqno,
                                                            const std::string& timeline) {
  DmaFenceSignaledPerfEvent event{
      static_cast<uint32_t>(sizeof(dma_fence_signaled_tracepoint) + timeline.length() + 1)};
  event.ring_buffer_record.sample_id.time = timestamp_ns;
  CHECK(event.GetTimestamp() == timestamp_ns);
  reinterpret_cast<dma_fence_signaled_tracepoint*>(event.tracepoint_data.get())->context = context;
  CHECK(event.GetContext() == context);
  reinterpret_cast<dma_fence_signaled_tracepoint*>(event.tracepoint_data.get())->seqno = seqno;
  CHECK(event.GetSeqno() == seqno);
  reinterpret_cast<dma_fence_signaled_tracepoint*>(event.tracepoint_data.get())->timeline =
      ((timeline.length() + 1) << 16) | sizeof(dma_fence_signaled_tracepoint);
  memcpy(event.tracepoint_data.get() + sizeof(dma_fence_signaled_tracepoint), timeline.c_str(),
         timeline.length() + 1);
  CHECK(event.ExtractTimelineString() == timeline);
  return event;
}

orbit_grpc_protos::GpuJob MakeGpuJob(int32_t tid, uint32_t context, uint32_t seqno,
                                     std::string timeline, int32_t depth,
                                     uint64_t amdgpu_cs_ioctl_time_ns,
                                     uint64_t amdgpu_sched_run_job_time_ns,
                                     uint64_t gpu_hardware_start_time_ns,
                                     uint64_t dma_fence_signaled_time_ns) {
  orbit_grpc_protos::GpuJob expected_gpu_job;
  expected_gpu_job.set_tid(tid);
  expected_gpu_job.set_context(context);
  expected_gpu_job.set_seqno(seqno);
  expected_gpu_job.set_timeline(std::move(timeline));
  expected_gpu_job.set_depth(depth);
  expected_gpu_job.set_amdgpu_cs_ioctl_time_ns(amdgpu_cs_ioctl_time_ns);
  expected_gpu_job.set_amdgpu_sched_run_job_time_ns(amdgpu_sched_run_job_time_ns);
  expected_gpu_job.set_gpu_hardware_start_time_ns(gpu_hardware_start_time_ns);
  expected_gpu_job.set_dma_fence_signaled_time_ns(dma_fence_signaled_time_ns);
  return expected_gpu_job;
}

::testing::Matcher<orbit_grpc_protos::GpuJob> GpuJobEq(const orbit_grpc_protos::GpuJob& expected) {
  return ::testing::AllOf(
      ::testing::Property("tid", &orbit_grpc_protos::GpuJob::tid, expected.tid()),
      ::testing::Property("context", &orbit_grpc_protos::GpuJob::context, expected.context()),
      ::testing::Property("seqno", &orbit_grpc_protos::GpuJob::seqno, expected.seqno()),
      ::testing::Property("timeline", &orbit_grpc_protos::GpuJob::timeline, expected.timeline()),
      ::testing::Property("depth", &orbit_grpc_protos::GpuJob::depth, expected.depth()),
      ::testing::Property("amdgpu_cs_ioctl_time_ns",
                          &orbit_grpc_protos::GpuJob::amdgpu_cs_ioctl_time_ns,
                          expected.amdgpu_cs_ioctl_time_ns()),
      ::testing::Property("amdgpu_sched_run_job_time_ns",
                          &orbit_grpc_protos::GpuJob::amdgpu_sched_run_job_time_ns,
                          expected.amdgpu_sched_run_job_time_ns()),
      ::testing::Property("gpu_hardware_start_time_ns",
                          &orbit_grpc_protos::GpuJob::gpu_hardware_start_time_ns,
                          expected.gpu_hardware_start_time_ns()),
      ::testing::Property("dma_fence_signaled_time_ns",
                          &orbit_grpc_protos::GpuJob::dma_fence_signaled_time_ns,
                          expected.dma_fence_signaled_time_ns()));
}

}  // namespace

TEST(GpuTracepointEventProcessor, NeedsListener) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampD = 300;

  GpuTracepointEventProcessor processor;
  processor.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext, kSeqno, kTimeline));
  processor.PushEvent(MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, kTimeline));
  EXPECT_DEATH(processor.PushEvent(
                   MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline)),
               "listener_ != nullptr");
}

TEST_F(GpuTracepointEventProcessorTest, JobCreatedWithAllThreePerfEvents) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampC = kTimestampB;
  static constexpr uint64_t kTimestampD = 300;

  orbit_grpc_protos::GpuJob expected_gpu_job = MakeGpuJob(
      kTid, kContext, kSeqno, kTimeline, 0, kTimestampA, kTimestampB, kTimestampC, kTimestampD);
  orbit_grpc_protos::GpuJob actual_gpu_job;
  EXPECT_CALL(mock_listener_, OnGpuJob).Times(1).WillOnce(::testing::SaveArg<0>(&actual_gpu_job));
  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, kTimeline));
  processor_.PushEvent(MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline));
  EXPECT_THAT(actual_gpu_job, GpuJobEq(expected_gpu_job));
}

TEST_F(GpuTracepointEventProcessorTest, JobCreatedEvenWithOutOfOrderPerfEvents1) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampC = kTimestampB;
  static constexpr uint64_t kTimestampD = 300;

  orbit_grpc_protos::GpuJob expected_gpu_job = MakeGpuJob(
      kTid, kContext, kSeqno, kTimeline, 0, kTimestampA, kTimestampB, kTimestampC, kTimestampD);
  orbit_grpc_protos::GpuJob actual_gpu_job;
  EXPECT_CALL(mock_listener_, OnGpuJob).Times(1).WillOnce(::testing::SaveArg<0>(&actual_gpu_job));
  processor_.PushEvent(MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext, kSeqno, kTimeline));
  EXPECT_THAT(actual_gpu_job, GpuJobEq(expected_gpu_job));
}

TEST_F(GpuTracepointEventProcessorTest, JobCreatedEvenWithOutOfOrderPerfEvents2) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampC = kTimestampB;
  static constexpr uint64_t kTimestampD = 300;

  orbit_grpc_protos::GpuJob expected_gpu_job = MakeGpuJob(
      kTid, kContext, kSeqno, kTimeline, 0, kTimestampA, kTimestampB, kTimestampC, kTimestampD);
  orbit_grpc_protos::GpuJob actual_gpu_job;
  EXPECT_CALL(mock_listener_, OnGpuJob).Times(1).WillOnce(::testing::SaveArg<0>(&actual_gpu_job));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext, kSeqno, kTimeline));
  processor_.PushEvent(MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline));
  EXPECT_THAT(actual_gpu_job, GpuJobEq(expected_gpu_job));
}

TEST_F(GpuTracepointEventProcessorTest, NoJobBecauseOfMismatchingContext) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampD = 300;

  EXPECT_CALL(mock_listener_, OnGpuJob).Times(0);
  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext + 1, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, kTimeline));
  processor_.PushEvent(MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline));
}

TEST_F(GpuTracepointEventProcessorTest, NoJobBecauseOfMismatchingSeqno) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampD = 300;

  EXPECT_CALL(mock_listener_, OnGpuJob).Times(0);
  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno + 1, kTimeline));
  processor_.PushEvent(MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline));
}

TEST_F(GpuTracepointEventProcessorTest, NoJobBecauseOfMismatchingTimeline) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampD = 300;

  EXPECT_CALL(mock_listener_, OnGpuJob).Times(0);
  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline + "1"));
}

TEST_F(GpuTracepointEventProcessorTest, TwoNonOverlappingJobsWithSameDepthDifferingByContext) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext1 = 1;
  static constexpr uint32_t kContext2 = 2;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kNsDistanceForSameDepth = 1'000'000;
  static constexpr uint64_t kTimestampA2 = kNsDistanceForSameDepth + 300;
  static constexpr uint64_t kTimestampB2 = kNsDistanceForSameDepth + 400;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = kNsDistanceForSameDepth + 500;

  orbit_grpc_protos::GpuJob expected_gpu_job1 =
      MakeGpuJob(kTid, kContext1, kSeqno, kTimeline, 0, kTimestampA1, kTimestampB1, kTimestampC1,
                 kTimestampD1);
  orbit_grpc_protos::GpuJob expected_gpu_job2 =
      MakeGpuJob(kTid, kContext2, kSeqno, kTimeline, 0, kTimestampA2, kTimestampB2, kTimestampC2,
                 kTimestampD2);
  orbit_grpc_protos::GpuJob actual_gpu_job1;
  orbit_grpc_protos::GpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA1, kContext1, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext1, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext1, kSeqno, kTimeline));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA2, kContext2, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext2, kSeqno, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext2, kSeqno, kTimeline));

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointEventProcessorTest, TwoNonOverlappingJobsWithSameDepthDifferingBySeqno) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kNsDistanceForSameDepth = 1'000'000;
  static constexpr uint64_t kTimestampA2 = kNsDistanceForSameDepth + 300;
  static constexpr uint64_t kTimestampB2 = kNsDistanceForSameDepth + 400;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = kNsDistanceForSameDepth + 500;

  orbit_grpc_protos::GpuJob expected_gpu_job1 =
      MakeGpuJob(kTid, kContext, kSeqno1, kTimeline, 0, kTimestampA1, kTimestampB1, kTimestampC1,
                 kTimestampD1);
  orbit_grpc_protos::GpuJob expected_gpu_job2 =
      MakeGpuJob(kTid, kContext, kSeqno2, kTimeline, 0, kTimestampA2, kTimestampB2, kTimestampC2,
                 kTimestampD2);
  orbit_grpc_protos::GpuJob actual_gpu_job1;
  orbit_grpc_protos::GpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, kTimeline));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, kTimeline));

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointEventProcessorTest, TwoOverlappingJobsButOnDifferentTimelines) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string kTimeline1 = "timeline1";
  static const std::string kTimeline2 = "timeline2";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampC = kTimestampB;
  static constexpr uint64_t kTimestampD = 300;

  orbit_grpc_protos::GpuJob expected_gpu_job1 = MakeGpuJob(
      kTid, kContext, kSeqno, kTimeline1, 0, kTimestampA, kTimestampB, kTimestampC, kTimestampD);
  orbit_grpc_protos::GpuJob expected_gpu_job2 = MakeGpuJob(
      kTid, kContext, kSeqno, kTimeline2, 0, kTimestampA, kTimestampB, kTimestampC, kTimestampD);
  orbit_grpc_protos::GpuJob actual_gpu_job1;
  orbit_grpc_protos::GpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext, kSeqno, kTimeline1));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, kTimeline1));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline1));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA, kContext, kSeqno, kTimeline2));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, kTimeline2));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, kTimeline2));

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointEventProcessorTest, TwoNonOverlappingJobsWithDifferentDepthsBecauseOfSlack) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kTimestampA2 = 400;
  static constexpr uint64_t kTimestampB2 = 500;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = 600;

  orbit_grpc_protos::GpuJob expected_gpu_job1 =
      MakeGpuJob(kTid, kContext, kSeqno1, kTimeline, 0, kTimestampA1, kTimestampB1, kTimestampC1,
                 kTimestampD1);
  orbit_grpc_protos::GpuJob expected_gpu_job2 =
      MakeGpuJob(kTid, kContext, kSeqno2, kTimeline, 1, kTimestampA2, kTimestampB2, kTimestampC2,
                 kTimestampD2);
  orbit_grpc_protos::GpuJob actual_gpu_job1;
  orbit_grpc_protos::GpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, kTimeline));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, kTimeline));

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointEventProcessorTest, TwoOverlappingJobsWithImmediateHwExecution) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kTimestampA2 = 110;
  static constexpr uint64_t kTimestampB2 = 310;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = 410;

  orbit_grpc_protos::GpuJob expected_gpu_job1 =
      MakeGpuJob(kTid, kContext, kSeqno1, kTimeline, 0, kTimestampA1, kTimestampB1, kTimestampC1,
                 kTimestampD1);
  orbit_grpc_protos::GpuJob expected_gpu_job2 =
      MakeGpuJob(kTid, kContext, kSeqno2, kTimeline, 1, kTimestampA2, kTimestampB2, kTimestampC2,
                 kTimestampD2);
  orbit_grpc_protos::GpuJob actual_gpu_job1;
  orbit_grpc_protos::GpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, kTimeline));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, kTimeline));

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointEventProcessorTest, TwoOverlappingJobsWithDelayedHwExecution) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kTimestampA2 = 110;
  static constexpr uint64_t kTimestampB2 = 210;
  static constexpr uint64_t kTimestampC2 = kTimestampD1;
  static constexpr uint64_t kTimestampD2 = 400;

  orbit_grpc_protos::GpuJob expected_gpu_job1 =
      MakeGpuJob(kTid, kContext, kSeqno1, kTimeline, 0, kTimestampA1, kTimestampB1, kTimestampC1,
                 kTimestampD1);
  orbit_grpc_protos::GpuJob expected_gpu_job2 =
      MakeGpuJob(kTid, kContext, kSeqno2, kTimeline, 1, kTimestampA2, kTimestampB2, kTimestampC2,
                 kTimestampD2);
  orbit_grpc_protos::GpuJob actual_gpu_job1;
  orbit_grpc_protos::GpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, kTimeline));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, kTimeline));

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointEventProcessorTest,
       TwoNonOverlappingJobsWithWrongDepthsAndHardwareStartsBecauseReceivedOutOfOrder) {
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string kTimeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kNsDistanceForSameDepth = 1'000'000;
  static constexpr uint64_t kTimestampA2 = kNsDistanceForSameDepth + 300;
  static constexpr uint64_t kTimestampB2 = kNsDistanceForSameDepth + 400;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = kNsDistanceForSameDepth + 500;
  // This is the timestamp that ends up being wrong when the assumption that "dma_fence_signaled"
  // events are processed reasonably in order doesn't hold.
  static constexpr uint64_t kTimestampC1 = kTimestampD2;

  orbit_grpc_protos::GpuJob expected_gpu_job1 =
      MakeGpuJob(kTid, kContext, kSeqno1, kTimeline, 1, kTimestampA1, kTimestampB1, kTimestampC1,
                 kTimestampD1);
  orbit_grpc_protos::GpuJob expected_gpu_job2 =
      MakeGpuJob(kTid, kContext, kSeqno2, kTimeline, 0, kTimestampA2, kTimestampB2, kTimestampC2,
                 kTimestampD2);
  orbit_grpc_protos::GpuJob actual_gpu_job1;
  orbit_grpc_protos::GpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      // Save actual_gpu_job2 first as it's created first (its last PerfEvent is processed first).
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1));

  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuCsIoctlPerfEvent(kTid, kTimestampA2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, kTimeline));
  processor_.PushEvent(
      MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, kTimeline));

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

}  // namespace LinuxTracing
