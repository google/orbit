#ifndef ORBIT_LINUX_TRACING_EVENTS_H_
#define ORBIT_LINUX_TRACING_EVENTS_H_

#include <unistd.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace LinuxTracing {

class ContextSwitch {
 public:
  pid_t GetPid() const { return pid_; }
  pid_t GetTid() const { return tid_; }
  uint16_t GetCore() const { return core_; }
  uint64_t GetTimestampNs() const { return timestamp_ns_; }

 protected:
  ContextSwitch(pid_t pid, pid_t tid, uint16_t core, uint64_t timestamp_ns)
      : pid_(pid), tid_(tid), core_(core), timestamp_ns_(timestamp_ns) {}

 private:
  pid_t pid_;
  pid_t tid_;
  uint16_t core_;
  uint64_t timestamp_ns_;
};

class ContextSwitchIn : public ContextSwitch {
 public:
  ContextSwitchIn(pid_t pid, pid_t tid, uint16_t core, uint64_t timestamp_ns)
      : ContextSwitch(pid, tid, core, timestamp_ns) {}
};

class ContextSwitchOut : public ContextSwitch {
 public:
  ContextSwitchOut(pid_t pid, pid_t tid, uint16_t core, uint64_t timestamp_ns)
      : ContextSwitch(pid, tid, core, timestamp_ns) {}
};

class CallstackFrame {
 public:
  CallstackFrame(uint64_t pc, std::string function_name,
                 uint64_t function_offset, std::string map_name)
      : pc_(pc),
        function_name_(std::move(function_name)),
        function_offset_(function_offset),
        map_name_(std::move(map_name)) {}

  uint64_t GetPc() const { return pc_; }
  const std::string& GetFunctionName() const { return function_name_; }
  uint64_t GetFunctionOffset() const { return function_offset_; }
  const std::string& GetMapName() const { return map_name_; }

 private:
  uint64_t pc_;
  std::string function_name_;
  uint64_t function_offset_;
  std::string map_name_;
};

class Callstack {
 public:
  Callstack(pid_t tid, std::vector<CallstackFrame> frames,
            uint64_t timestamp_ns)
      : tid_(tid), frames_(std::move(frames)), timestamp_ns_(timestamp_ns) {}

  pid_t GetTid() const { return tid_; }
  const std::vector<CallstackFrame>& GetFrames() const { return frames_; }
  uint64_t GetTimestampNs() const { return timestamp_ns_; }

 private:
  pid_t tid_;
  std::vector<CallstackFrame> frames_;
  uint64_t timestamp_ns_;
};

class FunctionCall {
 public:
  FunctionCall(pid_t tid, uint64_t virtual_address, uint64_t begin_timestamp_ns,
               uint64_t end_timestamp_ns, uint32_t depth, uint64_t return_value)
      : tid_(tid),
        virtual_address_(virtual_address),
        begin_timestamp_ns_(begin_timestamp_ns),
        end_timestamp_ns_(end_timestamp_ns),
        depth_{depth},
        return_value_(return_value) {}

  pid_t GetTid() const { return tid_; }
  uint64_t GetVirtualAddress() const { return virtual_address_; }
  uint64_t GetBeginTimestampNs() const { return begin_timestamp_ns_; }
  uint64_t GetEndTimestampNs() const { return end_timestamp_ns_; }
  uint32_t GetDepth() const { return depth_; }
  uint64_t GetIntegerReturnValue() const { return return_value_; }

 private:
  pid_t tid_;
  uint64_t virtual_address_;
  uint64_t begin_timestamp_ns_;
  uint64_t end_timestamp_ns_;
  uint32_t depth_;
  uint64_t return_value_;
};

class GpuJob {
 public:
  GpuJob(pid_t tid, uint32_t context, uint32_t seqno, std::string timeline,
         int32_t depth, uint64_t amdgpu_cs_ioctl_time_ns,
         uint64_t amdgpu_sched_run_job_time_ns,
         uint64_t gpu_hardware_start_time_ns,
         uint64_t dma_fence_signaled_time_ns)
      : tid_(tid),
        context_(context),
        seqno_(seqno),
        timeline_(std::move(timeline)),
        depth_(depth),
        amdgpu_cs_ioctl_time_ns_(amdgpu_cs_ioctl_time_ns),
        amdgpu_sched_run_job_time_ns_(amdgpu_sched_run_job_time_ns),
        gpu_hardware_start_time_ns_(gpu_hardware_start_time_ns),
        dma_fence_signaled_time_ns_(dma_fence_signaled_time_ns) {}

  pid_t GetTid() const { return tid_; }

  uint32_t GetContext() const { return context_; }
  uint32_t GetSeqno() const { return seqno_; }
  std::string GetTimeline() const { return timeline_; }

  int32_t GetDepth() const { return depth_; }

  uint64_t GetAmdgpuCsIoctlTimeNs() const { return amdgpu_cs_ioctl_time_ns_; }

  uint64_t GetAmdgpuSchedRunJobTimeNs() const {
    return amdgpu_sched_run_job_time_ns_;
  }

  uint64_t GetGpuHardwareStartTimeNs() const {
    return gpu_hardware_start_time_ns_;
  }

  uint64_t GetDmaFenceSignaledTimeNs() const {
    return dma_fence_signaled_time_ns_;
  }

 private:
  pid_t tid_;

  uint32_t context_;
  uint32_t seqno_;
  std::string timeline_;
  int32_t depth_;

  uint64_t amdgpu_cs_ioctl_time_ns_;
  uint64_t amdgpu_sched_run_job_time_ns_;
  // We do not have an explicit event for the following timestamp. We
  // assume that, when the GPU queue corresponding to timeline is
  // not executing a job, that this job starts exactly when it is
  // scheduled by the driver. Otherwise, we assume it starts exactly
  // when the previous job has signaled that it is done. Since we do
  // not have an explicit signal here, this is the best we can do.
  uint64_t gpu_hardware_start_time_ns_;
  uint64_t dma_fence_signaled_time_ns_;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_EVENTS_H_
