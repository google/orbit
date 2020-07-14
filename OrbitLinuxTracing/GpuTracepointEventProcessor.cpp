// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GpuTracepointEventProcessor.h"

#include <string>
#include <tuple>
#include <vector>

namespace LinuxTracing {

namespace {

// Format is based on the content of the event's format file:
// /sys/kernel/debug/tracing/events/<category>/<name>/format
struct __attribute__((__packed__)) perf_event_amdgpu_cs_ioctl {
  uint16_t common_type;
  uint8_t common_flags;
  uint8_t common_preempt_count;
  int32_t common_pid;
  uint64_t sched_job_id;
  int32_t timeline;
  uint32_t context;
  uint32_t seqno;
  uint64_t dma_fence;  // This is an address.
  uint64_t ring_name;  // This is an address.
  uint32_t num_ibs;
};

struct __attribute__((__packed__)) perf_event_amdgpu_sched_run_job {
  uint16_t common_type;
  uint8_t common_flags;
  uint8_t common_preempt_count;
  int32_t common_pid;
  uint64_t sched_job_id;
  int32_t timeline;
  uint32_t context;
  uint32_t seqno;
  uint64_t ring_name;  // This is an address.
  uint32_t num_ibs;
};

struct __attribute__((__packed__)) perf_event_dma_fence_signaled {
  uint16_t common_type;
  uint8_t common_flags;
  uint8_t common_preempt_count;
  int32_t common_pid;
  int32_t driver;
  int32_t timeline;
  uint32_t context;
  uint32_t seqno;
};

}  // namespace

int GpuTracepointEventProcessor::ComputeDepthForEvent(
    const std::string& timeline, uint64_t start_timestamp,
    uint64_t end_timestamp) {
  if (timeline_to_latest_timestamp_per_depth_.count(timeline) == 0) {
    std::vector<uint64_t> vec;
    timeline_to_latest_timestamp_per_depth_.emplace(timeline, vec);
  }
  auto it = timeline_to_latest_timestamp_per_depth_.find(timeline);
  std::vector<uint64_t>& vec = it->second;

  for (size_t d = 0; d < vec.size(); ++d) {
    // We add a small amount of slack on each row of the GPU track timeline to
    // make sure events don't get too crowded.
    constexpr uint64_t slack_ns = 1 * 1000000;
    if (start_timestamp >= (vec[d] + slack_ns)) {
      vec[d] = end_timestamp;
      return d;
    }
  }

  // Note that this vector only grows in size until a certain maximum depth is
  // reached. Since there are only O(10) events per frame created, the depth
  // is not likely to grow to a very large size.
  vec.push_back(end_timestamp);
  return static_cast<int>(vec.size() - 1);
}

void GpuTracepointEventProcessor::CreateGpuExecutionEventIfComplete(
    const Key& key) {
  auto cs_it = amdgpu_cs_ioctl_events_.find(key);
  auto sched_it = amdgpu_sched_run_job_events_.find(key);
  auto dma_it = dma_fence_signaled_events_.find(key);

  // First check if we have received all three events that are needed
  // to complete a full GPU execution event. Otherwise, we need to
  // keep waiting for events for this context, seqno, and timeline.
  if (cs_it == amdgpu_cs_ioctl_events_.end() ||
      sched_it == amdgpu_sched_run_job_events_.end() ||
      dma_it == dma_fence_signaled_events_.end()) {
    return;
  }

  std::string timeline = cs_it->second.timeline;
  pid_t tid = cs_it->second.tid;

  // We assume that GPU jobs (command buffer submissions) immediately
  // start running on the hardware when they are scheduled by the
  // driver (this is the best we can do), *unless* there is already a
  // job running. We keep track of when jobs finish in
  // timeline_to_latest_dma_signal_. If a previous job is still running
  // at the timestamp of scheduling the current job, we push the start
  // time for starting on the hardware back.
  if (timeline_to_latest_dma_signal_.count(timeline) == 0) {
    timeline_to_latest_dma_signal_.emplace(timeline,
                                           dma_it->second.timestamp_ns);
  }
  auto it = timeline_to_latest_dma_signal_.find(timeline);
  // We do not have an explicit event for the following timestamp. We
  // assume that, when the GPU queue corresponding to timeline is
  // not executing a job, that this job starts exactly when it is
  // scheduled by the driver. Otherwise, we assume it starts exactly
  // when the previous job has signaled that it is done. Since we do
  // not have an explicit signal here, this is the best we can do.
  uint64_t hw_start_time = sched_it->second.timestamp_ns;
  if (hw_start_time < it->second) {
    hw_start_time = it->second;
  }

  int depth = ComputeDepthForEvent(timeline, cs_it->second.timestamp_ns,
                                   dma_it->second.timestamp_ns);
  GpuJob gpu_job;
  gpu_job.set_tid(tid);
  gpu_job.set_context(cs_it->second.context);
  gpu_job.set_seqno(cs_it->second.seqno);
  gpu_job.set_timeline(cs_it->second.timeline);
  gpu_job.set_depth(depth);
  gpu_job.set_amdgpu_cs_ioctl_time_ns(cs_it->second.timestamp_ns);
  gpu_job.set_amdgpu_sched_run_job_time_ns(sched_it->second.timestamp_ns);
  gpu_job.set_gpu_hardware_start_time_ns(hw_start_time);
  gpu_job.set_dma_fence_signaled_time_ns(dma_it->second.timestamp_ns);

  listener_->OnGpuJob(std::move(gpu_job));

  // We need to update the timestamp when the last GPU job so far seen
  // finishes on this timeline.
  it->second = std::max(it->second, dma_it->second.timestamp_ns);

  amdgpu_cs_ioctl_events_.erase(key);
  amdgpu_sched_run_job_events_.erase(key);
  dma_fence_signaled_events_.erase(key);
}

void GpuTracepointEventProcessor::PushEvent(
    const std::unique_ptr<RawSamplePerfEvent>& sample) {
  pid_t tid = sample->ring_buffer_record.sample_id.tid;
  uint64_t timestamp_ns = sample->ring_buffer_record.sample_id.time;
  int tp_id =
      static_cast<int>(*reinterpret_cast<const uint16_t*>(&sample->data[0]));

  // Handle the three different types of events that we can get from the GPU
  // driver tracepoints we are tracing. We allow for the possibility that these
  // events arrive out-of-order (which is something we have actually observed)
  // with the following approach: We record all three types of events in
  // different maps. Whenever a new event arrives, we add it to the
  // corresponding map and then try to create a complete GPU execution event.
  // This event is only be created when all three types of GPU events have been
  // received.
  if (tp_id == amdgpu_cs_ioctl_id_) {
    const perf_event_amdgpu_cs_ioctl* tracepoint_data =
        reinterpret_cast<const perf_event_amdgpu_cs_ioctl*>(&sample->data[0]);

    uint32_t context = tracepoint_data->context;
    uint32_t seqno = tracepoint_data->seqno;
    std::string timeline = ExtractTimelineString(tracepoint_data);

    AmdgpuCsIoctlEvent event{tid, timestamp_ns, context, seqno, timeline};
    Key key = std::make_tuple(context, seqno, timeline);

    amdgpu_cs_ioctl_events_.emplace(key, event);

    CreateGpuExecutionEventIfComplete(key);
  } else if (tp_id == amdgpu_sched_run_job_id_) {
    const perf_event_amdgpu_sched_run_job* tracepoint_data =
        reinterpret_cast<const perf_event_amdgpu_sched_run_job*>(
            &sample->data[0]);

    uint32_t context = tracepoint_data->context;
    uint32_t seqno = tracepoint_data->seqno;
    std::string timeline = ExtractTimelineString(tracepoint_data);

    AmdgpuSchedRunJobEvent event{timestamp_ns, context, seqno, timeline};
    Key key = std::make_tuple(context, seqno, timeline);

    amdgpu_sched_run_job_events_.emplace(key, event);
    CreateGpuExecutionEventIfComplete(key);
  } else if (tp_id == dma_fence_signaled_id_) {
    const perf_event_dma_fence_signaled* tracepoint_data =
        reinterpret_cast<const perf_event_dma_fence_signaled*>(
            &sample->data[0]);

    uint32_t context = tracepoint_data->context;
    uint32_t seqno = tracepoint_data->seqno;
    std::string timeline = ExtractTimelineString(tracepoint_data);

    DmaFenceSignaledEvent event{timestamp_ns, context, seqno, timeline};
    Key key = std::make_tuple(context, seqno, timeline);

    dma_fence_signaled_events_.emplace(key, event);
    CreateGpuExecutionEventIfComplete(key);
  } else {
    CHECK(false);
  }
}

void GpuTracepointEventProcessor::SetListener(TracerListener* listener) {
  listener_ = listener;
}

}  // namespace LinuxTracing
