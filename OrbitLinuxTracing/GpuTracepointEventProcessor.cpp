#include "GpuTracepointEventProcessor.h"

namespace LinuxTracing {

namespace {

// Format is based on the content the the event's format file:
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
    const std::string& timeline,
    uint64_t start_timestamp, uint64_t end_timestamp) {
  if (timeline_to_latest_timestamp_per_depth_.count(timeline) == 0) {
    std::vector<uint64_t> vec;
    timeline_to_latest_timestamp_per_depth_.emplace(timeline, vec);
  }
  auto it = timeline_to_latest_timestamp_per_depth_.find(timeline);
  std::vector<uint64_t>& vec = it->second;

  for (int d = 0; d < vec.size(); ++d) {
    // We add a small slack on each row of the GPU track timeline to
    // make sure events don't get too crowded.
    constexpr uint64_t slack_ns = 5 * 1000000;
    if (start_timestamp >= (vec[d] + slack_ns)) {
      vec[d] = end_timestamp;
      return d;
    }
  }
  vec.push_back(end_timestamp);
  return static_cast<int>(vec.size());
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

  // We assume that GPU jobs (command buffer submissions) immediately
  // start running on the hardware when they are scheduled by the
  // driver (this is the best we can do), *unless* there is already a
  // job running. We keep track of when jobs finish in
  // timeline_to_latest_dma_signal_. If a previous job is still running
  // at the timestamp of scheduling the current job, we push the start
  // time for starting on the hardware back.
  if (timeline_to_latest_dma_signal_.count(timeline) == 0) {
    timeline_to_latest_dma_signal_.emplace(
        timeline, dma_it->second.timestamp_ns);
  }
  auto it = timeline_to_latest_dma_signal_.find(timeline);
  uint64_t hw_start_time = sched_it->second.timestamp_ns;
  if (hw_start_time < it->second) {
    hw_start_time = it->second;
  }

  int depth = ComputeDepthForEvent(timeline, cs_it->second.timestamp_ns,
                                   dma_it->second.timestamp_ns);

  // TODO: Remove debug output and send the actual event to the listener.
  LOG("GPU event complete: %d, %d, %s\n",
      std::get<0>(key), std::get<1>(key), std::get<2>(key).c_str());

  // GpuExecutionEvent gpu_event();
  // listener_->OnGpuExecutionEvent(gpu_event);

  amdgpu_cs_ioctl_events_.erase(key);
  amdgpu_sched_run_job_events_.erase(key);
  dma_fence_signaled_events_.erase(key);
}

void GpuTracepointEventProcessor::PushEvent(
    const std::unique_ptr<PerfEventSampleRaw>& sample) {
  uint32_t tid = sample->ring_buffer_record.sample_id.tid;
  uint64_t timestamp_ns = sample->ring_buffer_record.sample_id.time;
  int tp_id =
      static_cast<int>(*reinterpret_cast<const uint16_t*>(&sample->data[0]));

  if (tp_id == amdgpu_cs_ioctl_id_) {
    const perf_event_amdgpu_cs_ioctl* tracepoint_data =
        reinterpret_cast<const perf_event_amdgpu_cs_ioctl*>(&sample->data[0]);

    uint32_t context = tracepoint_data->context;
    uint32_t seqno = tracepoint_data->seqno;
    std::string timeline = ExtractTimelineString(tracepoint_data);

    AmdgpuCsIoctlEvent event{timestamp_ns, context, seqno, timeline};
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

}  // namespace LinuxTracing
