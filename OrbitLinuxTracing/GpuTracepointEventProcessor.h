
#ifndef ORBIT_LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR
#define ORBIT_LINUX_TRACING_GPU_TRACEPOINT_EVENT_PROCESSOR

#include <tuple>

#include "OrbitLinuxTracing/TracerListener.h"
#include "PerfEvent.h"
#include "absl/container/flat_hash_map.h"

namespace LinuxTracing {

class GpuTracepointEventProcessor {
 public:
  GpuTracepointEventProcessor(int amdgpu_cs_ioctl_id,
                              int amdgpu_sched_run_job_id,
                              int dma_fence_signaled_id)
      : amdgpu_cs_ioctl_id_(amdgpu_cs_ioctl_id),
        amdgpu_sched_run_job_id_(amdgpu_sched_run_job_id),
        dma_fence_signaled_id_(dma_fence_signaled_id) {}

  void PushEvent(const std::unique_ptr<PerfEventSampleRaw>& sample);
  void SetListener(TracerListener* listener);

 private:
  // Keys are context, seqno, and timeline
  typedef std::tuple<uint32_t, uint32_t, std::string> Key;

  template <typename T>
  std::string ExtractTimelineString(const T* tracepoint_data) {
    int32_t data_loc = tracepoint_data->timeline;
    int16_t data_loc_size = static_cast<int16_t>(data_loc >> 16);
    int16_t data_loc_offset = static_cast<int16_t>(data_loc & 0x00ff);

    std::vector<char> data_loc_data(data_loc_size);
    std::memcpy(
        &data_loc_data[0],
        reinterpret_cast<const char*>(tracepoint_data) + data_loc_offset,
        data_loc_size);

    // While the string should be null terminated here, we make sure that it
    // actually is by adding a zero in the last position. In the case of
    // expected behavior, this is a no-op.
    data_loc_data[data_loc_data.size() - 1] = 0;
    return std::string(&data_loc_data[0]);
  }

  int ComputeDepthForEvent(const std::string& timeline,
                           uint64_t start_timestamp, uint64_t end_timestamp);

  void CreateGpuExecutionEventIfComplete(const Key& key);

  int amdgpu_cs_ioctl_id_ = 0;
  int amdgpu_sched_run_job_id_ = 0;
  int dma_fence_signaled_id_ = 0;

  TracerListener* listener_ = nullptr;

  struct AmdgpuCsIoctlEvent {
    pid_t tid;
    uint64_t timestamp_ns;
    uint32_t context;
    uint32_t seqno;
    std::string timeline;
  };
  absl::flat_hash_map<Key, AmdgpuCsIoctlEvent> amdgpu_cs_ioctl_events_;

  struct AmdgpuSchedRunJobEvent {
    uint64_t timestamp_ns;
    uint32_t context;
    uint32_t seqno;
    std::string timeline;
  };
  absl::flat_hash_map<Key, AmdgpuSchedRunJobEvent> amdgpu_sched_run_job_events_;

  struct DmaFenceSignaledEvent {
    uint64_t timestamp_ns;
    uint32_t context;
    uint32_t seqno;
    std::string timeline;
  };
  absl::flat_hash_map<Key, DmaFenceSignaledEvent> dma_fence_signaled_events_;

  absl::flat_hash_map<std::string, uint64_t> timeline_to_latest_dma_signal_;

  absl::flat_hash_map<std::string, std::vector<uint64_t>>
      timeline_to_latest_timestamp_per_depth_;
};

}  // namespace LinuxTracing

#endif
