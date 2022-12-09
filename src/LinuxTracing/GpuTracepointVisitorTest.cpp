// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <google/protobuf/stubs/port.h>
#include <gtest/gtest.h>
#include <sys/types.h>

#include <cstdint>
#include <string>
#include <utility>

#include "GpuTracepointVisitor.h"
#include "GrpcProtos/capture.pb.h"
#include "MockTracerListener.h"
#include "PerfEvent.h"

namespace orbit_linux_tracing {

namespace {

class GpuTracepointVisitorTest : public ::testing::Test {
 protected:
  MockTracerListener mock_listener_;
  GpuTracepointVisitor visitor_{&mock_listener_};
};

AmdgpuCsIoctlPerfEvent MakeFakeAmdgpuCsIoctlPerfEvent(pid_t pid, pid_t tid, uint64_t timestamp_ns,
                                                      uint32_t context, uint32_t seqno,
                                                      std::string timeline) {
  return AmdgpuCsIoctlPerfEvent{
      .timestamp = timestamp_ns,
      .data =
          {
              .pid = pid,
              .tid = tid,
              .context = context,
              .seqno = seqno,
              .timeline_string = std::move(timeline),
          },
  };
}

AmdgpuSchedRunJobPerfEvent MakeFakeAmdgpuSchedRunJobPerfEvent(uint64_t timestamp_ns,
                                                              uint32_t context, uint32_t seqno,
                                                              std::string timeline) {
  return AmdgpuSchedRunJobPerfEvent{
      .timestamp = timestamp_ns,
      .data =
          {
              .context = context,
              .seqno = seqno,
              .timeline_string = std::move(timeline),
          },
  };
}

DmaFenceSignaledPerfEvent MakeFakeDmaFenceSignaledPerfEvent(uint64_t timestamp_ns, uint32_t context,
                                                            uint32_t seqno, std::string timeline) {
  return DmaFenceSignaledPerfEvent{
      .timestamp = timestamp_ns,
      .data =
          {
              .context = context,
              .seqno = seqno,
              .timeline_string = std::move(timeline),
          },
  };
}

orbit_grpc_protos::FullGpuJob MakeGpuJob(uint32_t pid, uint32_t tid, uint32_t context,
                                         uint32_t seqno, std::string timeline, int32_t depth,
                                         uint64_t amdgpu_cs_ioctl_time_ns,
                                         uint64_t amdgpu_sched_run_job_time_ns,
                                         uint64_t gpu_hardware_start_time_ns,
                                         uint64_t dma_fence_signaled_time_ns) {
  orbit_grpc_protos::FullGpuJob expected_gpu_job;
  expected_gpu_job.set_pid(pid);
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

::testing::Matcher<orbit_grpc_protos::FullGpuJob> GpuJobEq(
    const orbit_grpc_protos::FullGpuJob& expected) {
  return ::testing::AllOf(
      ::testing::Property("pid", &orbit_grpc_protos::FullGpuJob::pid, expected.pid()),
      ::testing::Property("tid", &orbit_grpc_protos::FullGpuJob::tid, expected.tid()),
      ::testing::Property("context", &orbit_grpc_protos::FullGpuJob::context, expected.context()),
      ::testing::Property("seqno", &orbit_grpc_protos::FullGpuJob::seqno, expected.seqno()),
      ::testing::Property("timeline", &orbit_grpc_protos::FullGpuJob::timeline,
                          expected.timeline()),
      ::testing::Property("depth", &orbit_grpc_protos::FullGpuJob::depth, expected.depth()),
      ::testing::Property("amdgpu_cs_ioctl_time_ns",
                          &orbit_grpc_protos::FullGpuJob::amdgpu_cs_ioctl_time_ns,
                          expected.amdgpu_cs_ioctl_time_ns()),
      ::testing::Property("amdgpu_sched_run_job_time_ns",
                          &orbit_grpc_protos::FullGpuJob::amdgpu_sched_run_job_time_ns,
                          expected.amdgpu_sched_run_job_time_ns()),
      ::testing::Property("gpu_hardware_start_time_ns",
                          &orbit_grpc_protos::FullGpuJob::gpu_hardware_start_time_ns,
                          expected.gpu_hardware_start_time_ns()),
      ::testing::Property("dma_fence_signaled_time_ns",
                          &orbit_grpc_protos::FullGpuJob::dma_fence_signaled_time_ns,
                          expected.dma_fence_signaled_time_ns()));
}

}  // namespace

TEST(GpuTracepointVisitor, NeedsListener) {
  EXPECT_DEATH(GpuTracepointVisitor{nullptr}, "listener_ != nullptr");
}

TEST_F(GpuTracepointVisitorTest, JobCreatedWithAllThreePerfEvents) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampC = kTimestampB;
  static constexpr uint64_t kTimestampD = 300;

  orbit_grpc_protos::FullGpuJob expected_gpu_job =
      MakeGpuJob(kPid, kTid, kContext, kSeqno, timeline, 0, kTimestampA, kTimestampB, kTimestampC,
                 kTimestampD);
  orbit_grpc_protos::FullGpuJob actual_gpu_job;
  EXPECT_CALL(mock_listener_, OnGpuJob).Times(1).WillOnce(::testing::SaveArg<0>(&actual_gpu_job));
  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA, kContext, kSeqno, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, timeline)}.Accept(
      &visitor_);

  EXPECT_THAT(actual_gpu_job, GpuJobEq(expected_gpu_job));
}

TEST_F(GpuTracepointVisitorTest, JobCreatedEvenWithOutOfOrderPerfEvents1) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampC = kTimestampB;
  static constexpr uint64_t kTimestampD = 300;

  orbit_grpc_protos::FullGpuJob expected_gpu_job =
      MakeGpuJob(kPid, kTid, kContext, kSeqno, timeline, 0, kTimestampA, kTimestampB, kTimestampC,
                 kTimestampD);
  orbit_grpc_protos::FullGpuJob actual_gpu_job;
  EXPECT_CALL(mock_listener_, OnGpuJob).Times(1).WillOnce(::testing::SaveArg<0>(&actual_gpu_job));
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, timeline)}.Accept(
      &visitor_);

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA, kContext, kSeqno, timeline)}
      .Accept(&visitor_);
  EXPECT_THAT(actual_gpu_job, GpuJobEq(expected_gpu_job));
}

TEST_F(GpuTracepointVisitorTest, JobCreatedEvenWithOutOfOrderPerfEvents2) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampC = kTimestampB;
  static constexpr uint64_t kTimestampD = 300;

  orbit_grpc_protos::FullGpuJob expected_gpu_job =
      MakeGpuJob(kPid, kTid, kContext, kSeqno, timeline, 0, kTimestampA, kTimestampB, kTimestampC,
                 kTimestampD);
  orbit_grpc_protos::FullGpuJob actual_gpu_job;
  EXPECT_CALL(mock_listener_, OnGpuJob).Times(1).WillOnce(::testing::SaveArg<0>(&actual_gpu_job));
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA, kContext, kSeqno, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, timeline)}.Accept(
      &visitor_);
  EXPECT_THAT(actual_gpu_job, GpuJobEq(expected_gpu_job));
}

TEST_F(GpuTracepointVisitorTest, NoJobBecauseOfMismatchingContext) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampD = 300;

  EXPECT_CALL(mock_listener_, OnGpuJob).Times(0);

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA, kContext + 1, kSeqno, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, timeline)}.Accept(
      &visitor_);
}

TEST_F(GpuTracepointVisitorTest, NoJobBecauseOfMismatchingSeqno) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampD = 300;

  EXPECT_CALL(mock_listener_, OnGpuJob).Times(0);
  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA, kContext, kSeqno, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno + 1, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, timeline)}.Accept(
      &visitor_);
}

TEST_F(GpuTracepointVisitorTest, NoJobBecauseOfMismatchingTimeline) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampD = 300;

  EXPECT_CALL(mock_listener_, OnGpuJob).Times(0);
  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA, kContext, kSeqno, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, timeline + "1")}
      .Accept(&visitor_);
}

TEST_F(GpuTracepointVisitorTest, TwoNonOverlappingJobsWithSameDepthDifferingByContext) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext1 = 1;
  static constexpr uint32_t kContext2 = 2;
  static constexpr uint32_t kSeqno = 10;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kNsDistanceForSameDepth = 1'000'000;
  static constexpr uint64_t kTimestampA2 = kNsDistanceForSameDepth + 300;
  static constexpr uint64_t kTimestampB2 = kNsDistanceForSameDepth + 400;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = kNsDistanceForSameDepth + 500;

  orbit_grpc_protos::FullGpuJob expected_gpu_job1 =
      MakeGpuJob(kPid, kTid, kContext1, kSeqno, timeline, 0, kTimestampA1, kTimestampB1,
                 kTimestampC1, kTimestampD1);
  orbit_grpc_protos::FullGpuJob expected_gpu_job2 =
      MakeGpuJob(kPid, kTid, kContext2, kSeqno, timeline, 0, kTimestampA2, kTimestampB2,
                 kTimestampC2, kTimestampD2);
  orbit_grpc_protos::FullGpuJob actual_gpu_job1;
  orbit_grpc_protos::FullGpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA1, kContext1, kSeqno, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext1, kSeqno, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext1, kSeqno, timeline)}.Accept(
      &visitor_);

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA2, kContext2, kSeqno, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext2, kSeqno, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext2, kSeqno, timeline)}.Accept(
      &visitor_);

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointVisitorTest, TwoNonOverlappingJobsWithSameDepthDifferingBySeqno) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kNsDistanceForSameDepth = 1'000'000;
  static constexpr uint64_t kTimestampA2 = kNsDistanceForSameDepth + 300;
  static constexpr uint64_t kTimestampB2 = kNsDistanceForSameDepth + 400;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = kNsDistanceForSameDepth + 500;

  orbit_grpc_protos::FullGpuJob expected_gpu_job1 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno1, timeline, 0, kTimestampA1, kTimestampB1,
                 kTimestampC1, kTimestampD1);
  orbit_grpc_protos::FullGpuJob expected_gpu_job2 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno2, timeline, 0, kTimestampA2, kTimestampB2,
                 kTimestampC2, kTimestampD2);
  orbit_grpc_protos::FullGpuJob actual_gpu_job1;
  orbit_grpc_protos::FullGpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA1, kContext, kSeqno1, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA2, kContext, kSeqno2, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointVisitorTest, TwoOverlappingJobsButOnDifferentTimelines) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno = 10;
  static const std::string timeline1 = "timeline1";
  static const std::string timeline2 = "timeline2";
  static constexpr uint64_t kTimestampA = 100;
  static constexpr uint64_t kTimestampB = 200;
  static constexpr uint64_t kTimestampC = kTimestampB;
  static constexpr uint64_t kTimestampD = 300;

  orbit_grpc_protos::FullGpuJob expected_gpu_job1 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno, timeline1, 0, kTimestampA, kTimestampB, kTimestampC,
                 kTimestampD);
  orbit_grpc_protos::FullGpuJob expected_gpu_job2 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno, timeline2, 0, kTimestampA, kTimestampB, kTimestampC,
                 kTimestampD);
  orbit_grpc_protos::FullGpuJob actual_gpu_job1;
  orbit_grpc_protos::FullGpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA, kContext, kSeqno, timeline1)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, timeline1)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, timeline1)}.Accept(
      &visitor_);

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA, kContext, kSeqno, timeline2)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB, kContext, kSeqno, timeline2)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD, kContext, kSeqno, timeline2)}.Accept(
      &visitor_);

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointVisitorTest, TwoNonOverlappingJobsWithDifferentDepthsBecauseOfSlack) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kTimestampA2 = 400;
  static constexpr uint64_t kTimestampB2 = 500;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = 600;

  orbit_grpc_protos::FullGpuJob expected_gpu_job1 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno1, timeline, 0, kTimestampA1, kTimestampB1,
                 kTimestampC1, kTimestampD1);
  orbit_grpc_protos::FullGpuJob expected_gpu_job2 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno2, timeline, 1, kTimestampA2, kTimestampB2,
                 kTimestampC2, kTimestampD2);
  orbit_grpc_protos::FullGpuJob actual_gpu_job1;
  orbit_grpc_protos::FullGpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA1, kContext, kSeqno1, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA2, kContext, kSeqno2, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointVisitorTest, TwoOverlappingJobsWithImmediateHwExecution) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kTimestampA2 = 110;
  static constexpr uint64_t kTimestampB2 = 310;
  static constexpr uint64_t kTimestampC2 = kTimestampB2;
  static constexpr uint64_t kTimestampD2 = 410;

  orbit_grpc_protos::FullGpuJob expected_gpu_job1 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno1, timeline, 0, kTimestampA1, kTimestampB1,
                 kTimestampC1, kTimestampD1);
  orbit_grpc_protos::FullGpuJob expected_gpu_job2 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno2, timeline, 1, kTimestampA2, kTimestampB2,
                 kTimestampC2, kTimestampD2);
  orbit_grpc_protos::FullGpuJob actual_gpu_job1;
  orbit_grpc_protos::FullGpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA1, kContext, kSeqno1, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA2, kContext, kSeqno2, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointVisitorTest, TwoOverlappingJobsWithDelayedHwExecution) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string timeline = "timeline";
  static constexpr uint64_t kTimestampA1 = 100;
  static constexpr uint64_t kTimestampB1 = 200;
  static constexpr uint64_t kTimestampC1 = kTimestampB1;
  static constexpr uint64_t kTimestampD1 = 300;
  static constexpr uint64_t kTimestampA2 = 110;
  static constexpr uint64_t kTimestampB2 = 210;
  static constexpr uint64_t kTimestampC2 = kTimestampD1;
  static constexpr uint64_t kTimestampD2 = 400;

  orbit_grpc_protos::FullGpuJob expected_gpu_job1 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno1, timeline, 0, kTimestampA1, kTimestampB1,
                 kTimestampC1, kTimestampD1);
  orbit_grpc_protos::FullGpuJob expected_gpu_job2 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno2, timeline, 1, kTimestampA2, kTimestampB2,
                 kTimestampC2, kTimestampD2);
  orbit_grpc_protos::FullGpuJob actual_gpu_job1;
  orbit_grpc_protos::FullGpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2));

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA1, kContext, kSeqno1, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA2, kContext, kSeqno2, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

TEST_F(GpuTracepointVisitorTest,
       TwoNonOverlappingJobsWithWrongDepthsAndHardwareStartsBecauseReceivedOutOfOrder) {
  static constexpr pid_t kPid = 41;
  static constexpr pid_t kTid = 42;
  static constexpr uint32_t kContext = 1;
  static constexpr uint32_t kSeqno1 = 10;
  static constexpr uint32_t kSeqno2 = 20;
  static const std::string timeline = "timeline";
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

  orbit_grpc_protos::FullGpuJob expected_gpu_job1 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno1, timeline, 1, kTimestampA1, kTimestampB1,
                 kTimestampC1, kTimestampD1);
  orbit_grpc_protos::FullGpuJob expected_gpu_job2 =
      MakeGpuJob(kPid, kTid, kContext, kSeqno2, timeline, 0, kTimestampA2, kTimestampB2,
                 kTimestampC2, kTimestampD2);
  orbit_grpc_protos::FullGpuJob actual_gpu_job1;
  orbit_grpc_protos::FullGpuJob actual_gpu_job2;
  EXPECT_CALL(mock_listener_, OnGpuJob)
      .Times(2)
      // Save actual_gpu_job2 first as it's created first (its last PerfEvent is processed first).
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job2))
      .WillOnce(::testing::SaveArg<0>(&actual_gpu_job1));

  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA1, kContext, kSeqno1, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeAmdgpuCsIoctlPerfEvent(kPid, kTid, kTimestampA2, kContext, kSeqno2, timeline)}
      .Accept(&visitor_);
  PerfEvent{MakeFakeAmdgpuSchedRunJobPerfEvent(kTimestampB2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD2, kContext, kSeqno2, timeline)}.Accept(
      &visitor_);
  PerfEvent{MakeFakeDmaFenceSignaledPerfEvent(kTimestampD1, kContext, kSeqno1, timeline)}.Accept(
      &visitor_);

  EXPECT_THAT(actual_gpu_job1, GpuJobEq(expected_gpu_job1));
  EXPECT_THAT(actual_gpu_job2, GpuJobEq(expected_gpu_job2));
}

}  // namespace orbit_linux_tracing
