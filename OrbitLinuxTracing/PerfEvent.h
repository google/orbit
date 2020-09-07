// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_PERF_EVENT_H_
#define ORBIT_LINUX_TRACING_PERF_EVENT_H_

#include <array>
#include <memory>

#include "Function.h"
#include "KernelTracepoints.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "PerfEventRecords.h"

namespace LinuxTracing {

class PerfEventVisitor;

// This base class is used to do processing of different perf_event_open events
// using the visitor pattern. To avoid unnecessary copies, the data of the
// perf_event_open records will be copied from the ring buffer directly into the
// concrete subclass (depending on the event type), in general into a
// "ring_buffer_record" field.

class PerfEvent {
 public:
  virtual ~PerfEvent() = default;
  virtual uint64_t GetTimestamp() const = 0;
  virtual void Accept(PerfEventVisitor* visitor) = 0;
  void SetOriginFileDescriptor(int fd) { origin_file_descriptor_ = fd; }
  int GetOriginFileDescriptor() const { return origin_file_descriptor_; }

 private:
  int origin_file_descriptor_ = -1;
};

class ContextSwitchPerfEvent : public PerfEvent {
 public:
  perf_event_context_switch ring_buffer_record;

  uint64_t GetTimestamp() const override { return ring_buffer_record.sample_id.time; }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }

  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  bool IsSwitchOut() const { return ring_buffer_record.header.misc & PERF_RECORD_MISC_SWITCH_OUT; }
  bool IsSwitchIn() const { return !IsSwitchOut(); }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

class SystemWideContextSwitchPerfEvent : public PerfEvent {
 public:
  perf_event_context_switch_cpu_wide ring_buffer_record;

  uint64_t GetTimestamp() const override { return ring_buffer_record.sample_id.time; }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }

  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  bool IsSwitchOut() const { return ring_buffer_record.header.misc & PERF_RECORD_MISC_SWITCH_OUT; }

  bool IsSwitchIn() const { return !IsSwitchOut(); }

  // Careful: even if PERF_RECORD_SWITCH_CPU_WIDE events carry information on
  // both the thread being de-scheduled and the one being scheduled (if the cpu
  // is switching from a thread to another and not from/to an idle state), two
  // separate PERF_RECORD_SWITCH_CPU_WIDE are still generated, one for the
  // switch-out and one for the switch-in. Therefore, prefer GetPid/Tid and
  // IsSwitchOut/In to GetPrev/NextPid/Tid.

  pid_t GetPrevPid() const { return IsSwitchOut() ? GetPid() : ring_buffer_record.next_prev_pid; }

  pid_t GetPrevTid() const { return IsSwitchOut() ? GetTid() : ring_buffer_record.next_prev_tid; }

  pid_t GetNextPid() const { return IsSwitchOut() ? ring_buffer_record.next_prev_pid : GetPid(); }

  pid_t GetNextTid() const { return IsSwitchOut() ? ring_buffer_record.next_prev_tid : GetTid(); }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

class ForkPerfEvent : public PerfEvent {
 public:
  perf_event_fork_exit ring_buffer_record;

  uint64_t GetTimestamp() const override { return ring_buffer_record.time; }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.pid; }
  pid_t GetParentPid() const { return ring_buffer_record.ppid; }
  pid_t GetTid() const { return ring_buffer_record.tid; }
  pid_t GetParentTid() const { return ring_buffer_record.ptid; }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

class ExitPerfEvent : public PerfEvent {
 public:
  perf_event_fork_exit ring_buffer_record;

  uint64_t GetTimestamp() const override { return ring_buffer_record.time; }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.pid; }
  pid_t GetParentPid() const { return ring_buffer_record.ppid; }
  pid_t GetTid() const { return ring_buffer_record.tid; }
  pid_t GetParentTid() const { return ring_buffer_record.ptid; }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

class LostPerfEvent : public PerfEvent {
 public:
  perf_event_lost ring_buffer_record;

  uint64_t GetTimestamp() const override { return ring_buffer_record.sample_id.time; }

  void Accept(PerfEventVisitor* visitor) override;

  uint64_t GetNumLost() const { return ring_buffer_record.lost; }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

struct dynamically_sized_perf_event_stack_sample {
  struct dynamically_sized_perf_event_sample_stack_user {
    uint64_t dyn_size;
    std::unique_ptr<char[]> data;

    explicit dynamically_sized_perf_event_sample_stack_user(uint64_t dyn_size)
        : dyn_size{dyn_size}, data{make_unique_for_overwrite<char[]>(dyn_size)} {}
  };

  perf_event_header header;
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
  perf_event_sample_regs_user_all regs;
  dynamically_sized_perf_event_sample_stack_user stack;

  explicit dynamically_sized_perf_event_stack_sample(uint64_t dyn_size) : stack{dyn_size} {}
};

class StackSamplePerfEvent : public PerfEvent {
 public:
  std::unique_ptr<dynamically_sized_perf_event_stack_sample> ring_buffer_record;

  explicit StackSamplePerfEvent(uint64_t dyn_size)
      : ring_buffer_record{std::make_unique<dynamically_sized_perf_event_stack_sample>(dyn_size)} {}

  uint64_t GetTimestamp() const override { return ring_buffer_record->sample_id.time; }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record->sample_id.pid; }
  pid_t GetTid() const { return ring_buffer_record->sample_id.tid; }

  uint64_t GetStreamId() const { return ring_buffer_record->sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record->sample_id.cpu; }

  std::array<uint64_t, PERF_REG_X86_64_MAX> GetRegisters() const {
    return perf_event_sample_regs_user_all_to_register_array(ring_buffer_record->regs);
  }

  const char* GetStackData() const { return ring_buffer_record->stack.data.get(); }
  char* GetStackData() { return ring_buffer_record->stack.data.get(); }
  uint64_t GetStackSize() const { return ring_buffer_record->stack.dyn_size; }

 private:
  static std::array<uint64_t, PERF_REG_X86_64_MAX>
  perf_event_sample_regs_user_all_to_register_array(const perf_event_sample_regs_user_all& regs) {
    std::array<uint64_t, PERF_REG_X86_64_MAX> registers{};
    registers[PERF_REG_X86_AX] = regs.ax;
    registers[PERF_REG_X86_BX] = regs.bx;
    registers[PERF_REG_X86_CX] = regs.cx;
    registers[PERF_REG_X86_DX] = regs.dx;
    registers[PERF_REG_X86_SI] = regs.si;
    registers[PERF_REG_X86_DI] = regs.di;
    registers[PERF_REG_X86_BP] = regs.bp;
    registers[PERF_REG_X86_SP] = regs.sp;
    registers[PERF_REG_X86_IP] = regs.ip;
    registers[PERF_REG_X86_FLAGS] = regs.flags;
    registers[PERF_REG_X86_CS] = regs.cs;
    registers[PERF_REG_X86_SS] = regs.ss;
    // Registers ds, es, fs, gs do not actually exist.
    registers[PERF_REG_X86_DS] = 0ul;
    registers[PERF_REG_X86_ES] = 0ul;
    registers[PERF_REG_X86_FS] = 0ul;
    registers[PERF_REG_X86_GS] = 0ul;
    registers[PERF_REG_X86_R8] = regs.r8;
    registers[PERF_REG_X86_R9] = regs.r9;
    registers[PERF_REG_X86_R10] = regs.r10;
    registers[PERF_REG_X86_R11] = regs.r11;
    registers[PERF_REG_X86_R12] = regs.r12;
    registers[PERF_REG_X86_R13] = regs.r13;
    registers[PERF_REG_X86_R14] = regs.r14;
    registers[PERF_REG_X86_R15] = regs.r15;
    return registers;
  }
};

class CallchainSamplePerfEvent : public PerfEvent {
 public:
  perf_event_callchain_sample_fixed ring_buffer_record;
  std::vector<uint64_t> ips;
  explicit CallchainSamplePerfEvent(uint64_t callchain_size) : ips(callchain_size) {
    ring_buffer_record.nr = callchain_size;
  }

  uint64_t GetTimestamp() const override { return ring_buffer_record.sample_id.time; }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }
  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }

  uint64_t* GetCallchain() { return ips.data(); }
  const uint64_t* GetCallchain() const { return ips.data(); }

  uint64_t GetCallchainSize() const { return ring_buffer_record.nr; }
};

class AbstractUprobesPerfEvent {
 public:
  const Function* GetFunction() const { return function_; }
  void SetFunction(const Function* function) { function_ = function; }

 private:
  const Function* function_ = nullptr;
};

class UprobesPerfEvent : public PerfEvent, public AbstractUprobesPerfEvent {
 public:
  perf_event_sp_ip_arguments_8bytes_sample ring_buffer_record;

  uint64_t GetTimestamp() const override { return ring_buffer_record.sample_id.time; }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }
  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }

  // Get the stack pointer.
  uint64_t GetSp() const { return ring_buffer_record.regs.sp; }

  // Get the instruction pointer.
  uint64_t GetIp() const { return ring_buffer_record.regs.ip; }

  uint64_t GetReturnAddress() const { return ring_buffer_record.stack.top8bytes; }
};

class UretprobesPerfEvent : public PerfEvent, public AbstractUprobesPerfEvent {
 public:
  perf_event_ax_sample ring_buffer_record;

  uint64_t GetTimestamp() const override { return ring_buffer_record.sample_id.time; }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }
  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  // Get AX register which holds integer return value.
  // See https://wiki.osdev.org/System_V_ABI.
  uint64_t GetAx() const { return ring_buffer_record.regs.ax; }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

// This carries a snapshot of /proc/<pid>/maps and does not reflect a
// perf_event_open event, but we want it to be part of the same hierarchy.
class MapsPerfEvent : public PerfEvent {
 public:
  MapsPerfEvent(uint64_t timestamp, std::string maps)
      : timestamp_{timestamp}, maps_{std::move(maps)} {}

  uint64_t GetTimestamp() const override { return timestamp_; }

  void Accept(PerfEventVisitor* visitor) override;

  const std::string& GetMaps() const { return maps_; }

 private:
  uint64_t timestamp_;
  std::string maps_;
};

class TracepointPerfEvent : public PerfEvent {
 public:
  explicit TracepointPerfEvent(uint32_t size)
      : tracepoint_data{make_unique_for_overwrite<uint8_t[]>(size)} {}

  perf_event_raw_sample_fixed ring_buffer_record;
  std::unique_ptr<uint8_t[]> tracepoint_data;

  uint64_t GetTimestamp() const override { return ring_buffer_record.sample_id.time; }

  uint64_t GetStreamId() const { return ring_buffer_record.sample_id.stream_id; }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }

  uint16_t GetTracepointId() const { return GetTracepointCommon().common_type; }

 protected:
  const tracepoint_common& GetTracepointCommon() const {
    return *reinterpret_cast<const tracepoint_common*>(tracepoint_data.get());
  }

  template <typename TracepointData>
  const TracepointData& GetTypedTracepointData() const {
    return *reinterpret_cast<const TracepointData*>(tracepoint_data.get());
  }
};

class TaskNewtaskPerfEvent : public TracepointPerfEvent {
 public:
  explicit TaskNewtaskPerfEvent(uint32_t tracepoint_size) : TracepointPerfEvent(tracepoint_size) {}

  void Accept(PerfEventVisitor* visitor) override;

  // The tracepoint format calls this "pid" but it's effectively the thread id.
  pid_t GetTid() const { return GetTypedTracepointData<task_newtask_tracepoint>().pid; }

  const char* GetComm() const { return GetTypedTracepointData<task_newtask_tracepoint>().comm; }
};

class TaskRenamePerfEvent : public TracepointPerfEvent {
 public:
  explicit TaskRenamePerfEvent(uint32_t tracepoint_size) : TracepointPerfEvent(tracepoint_size) {}

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetTid() const { return GetTypedTracepointData<task_rename_tracepoint>().pid; }

  const char* GetOldComm() const {
    return GetTypedTracepointData<task_rename_tracepoint>().oldcomm;
  }
  const char* GetNewComm() const {
    return GetTypedTracepointData<task_rename_tracepoint>().newcomm;
  }
};

class TracepointEventPidTidTimeCpu {
 public:
  explicit TracepointEventPidTidTimeCpu() {}

  perf_event_sample_id_tid_time_streamid_cpu ring_buffer_record;

  int32_t GetPid() const { return ring_buffer_record.pid; }

  int32_t GetTid() const { return ring_buffer_record.tid; }

  uint64_t GetTimestamp() const { return ring_buffer_record.time; }

  uint32_t GetCpu() const { return ring_buffer_record.cpu; }
};

class GpuPerfEvent : public TracepointPerfEvent {
 public:
  explicit GpuPerfEvent(uint32_t tracepoint_size) : TracepointPerfEvent(tracepoint_size) {}

  std::string ExtractTimelineString() const {
    int32_t data_loc = GetTimeline();
    int16_t data_loc_size = static_cast<int16_t>(data_loc >> 16);
    int16_t data_loc_offset = static_cast<int16_t>(data_loc & 0x00ff);

    std::vector<char> data_loc_data(data_loc_size);
    std::memcpy(&data_loc_data[0],
                reinterpret_cast<const char*>(tracepoint_data.get()) + data_loc_offset,
                data_loc_size);

    // While the string should be null terminated here, we make sure that it
    // actually is by adding a zero in the last position. In the case of
    // expected behavior, this is a no-op.
    data_loc_data[data_loc_data.size() - 1] = 0;
    return std::string(&data_loc_data[0]);
  }

  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  virtual uint32_t GetContext() const = 0;
  virtual uint32_t GetSeqno() const = 0;

 protected:
  virtual int32_t GetTimeline() const = 0;
};

class AmdgpuCsIoctlPerfEvent : public GpuPerfEvent {
 public:
  explicit AmdgpuCsIoctlPerfEvent(uint32_t tracepoint_size) : GpuPerfEvent(tracepoint_size) {}

  void Accept(PerfEventVisitor* visitor) override;

  uint32_t GetContext() const override {
    return GetTypedTracepointData<amdgpu_cs_ioctl_tracepoint>().context;
  }

  uint32_t GetSeqno() const override {
    return GetTypedTracepointData<amdgpu_cs_ioctl_tracepoint>().seqno;
  }

 protected:
  int32_t GetTimeline() const override {
    return GetTypedTracepointData<amdgpu_cs_ioctl_tracepoint>().timeline;
  }
};

class AmdgpuSchedRunJobPerfEvent : public GpuPerfEvent {
 public:
  explicit AmdgpuSchedRunJobPerfEvent(uint32_t tracepoint_size) : GpuPerfEvent(tracepoint_size) {}

  void Accept(PerfEventVisitor* visitor) override;

  uint32_t GetContext() const override {
    return GetTypedTracepointData<amdgpu_sched_run_job_tracepoint>().context;
  }

  uint32_t GetSeqno() const override {
    return GetTypedTracepointData<amdgpu_sched_run_job_tracepoint>().seqno;
  }

 protected:
  int32_t GetTimeline() const override {
    return GetTypedTracepointData<amdgpu_sched_run_job_tracepoint>().timeline;
  }
};

class DmaFenceSignaledPerfEvent : public GpuPerfEvent {
 public:
  explicit DmaFenceSignaledPerfEvent(uint32_t tracepoint_size) : GpuPerfEvent(tracepoint_size) {}

  void Accept(PerfEventVisitor* visitor) override;

  uint32_t GetContext() const override {
    return GetTypedTracepointData<dma_fence_signaled_tracepoint>().context;
  }

  uint32_t GetSeqno() const override {
    return GetTypedTracepointData<dma_fence_signaled_tracepoint>().seqno;
  }

 protected:
  int32_t GetTimeline() const override {
    return GetTypedTracepointData<dma_fence_signaled_tracepoint>().timeline;
  }
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_H_
