#ifndef ORBIT_LINUX_TRACING_PERF_EVENT_H_
#define ORBIT_LINUX_TRACING_PERF_EVENT_H_

#include <OrbitLinuxTracing/Function.h>

#include <array>
#include <memory>

#include "MakeUniqueForOverwrite.h"
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

  uint64_t GetTimestamp() const override {
    return ring_buffer_record.sample_id.time;
  }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }

  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  bool IsSwitchOut() const {
    return ring_buffer_record.header.misc & PERF_RECORD_MISC_SWITCH_OUT;
  }
  bool IsSwitchIn() const { return !IsSwitchOut(); }

  uint64_t GetStreamId() const {
    return ring_buffer_record.sample_id.stream_id;
  }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

class SystemWideContextSwitchPerfEvent : public PerfEvent {
 public:
  perf_event_context_switch_cpu_wide ring_buffer_record;

  uint64_t GetTimestamp() const override {
    return ring_buffer_record.sample_id.time;
  }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }

  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  bool IsSwitchOut() const {
    return ring_buffer_record.header.misc & PERF_RECORD_MISC_SWITCH_OUT;
  }

  bool IsSwitchIn() const { return !IsSwitchOut(); }

  // Careful: even if PERF_RECORD_SWITCH_CPU_WIDE events carry information on
  // both the thread being de-scheduled and the one being scheduled (if the cpu
  // is switching from a thread to another and not from/to an idle state), two
  // separate PERF_RECORD_SWITCH_CPU_WIDE are still generated, one for the
  // switch-out and one for the switch-in. Therefore, prefer GetPid/Tid and
  // IsSwitchOut/In to GetPrev/NextPid/Tid.

  pid_t GetPrevPid() const {
    return IsSwitchOut() ? GetPid() : ring_buffer_record.next_prev_pid;
  }

  pid_t GetPrevTid() const {
    return IsSwitchOut() ? GetTid() : ring_buffer_record.next_prev_tid;
  }

  pid_t GetNextPid() const {
    return IsSwitchOut() ? ring_buffer_record.next_prev_pid : GetPid();
  }

  pid_t GetNextTid() const {
    return IsSwitchOut() ? ring_buffer_record.next_prev_tid : GetTid();
  }

  uint64_t GetStreamId() const {
    return ring_buffer_record.sample_id.stream_id;
  }

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

  uint64_t GetStreamId() const {
    return ring_buffer_record.sample_id.stream_id;
  }

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

  uint64_t GetStreamId() const {
    return ring_buffer_record.sample_id.stream_id;
  }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

class LostPerfEvent : public PerfEvent {
 public:
  perf_event_lost ring_buffer_record;

  uint64_t GetTimestamp() const override {
    return ring_buffer_record.sample_id.time;
  }

  void Accept(PerfEventVisitor* visitor) override;

  uint64_t GetNumLost() const { return ring_buffer_record.lost; }

  uint64_t GetStreamId() const {
    return ring_buffer_record.sample_id.stream_id;
  }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }
};

struct dynamically_sized_perf_event_stack_sample {
  struct dynamically_sized_perf_event_sample_stack_user {
    uint64_t dyn_size;
    std::unique_ptr<char[]> data;

    explicit dynamically_sized_perf_event_sample_stack_user(uint64_t dyn_size)
        : dyn_size{dyn_size},
          data{make_unique_for_overwrite<char[]>(dyn_size)} {}
  };

  perf_event_header header;
  perf_event_sample_id_tid_time_streamid_cpu sample_id;
  perf_event_sample_regs_user_all regs;
  dynamically_sized_perf_event_sample_stack_user stack;

  explicit dynamically_sized_perf_event_stack_sample(uint64_t dyn_size)
      : stack{dyn_size} {}
};

class SamplePerfEvent : public PerfEvent {
 public:
  std::unique_ptr<dynamically_sized_perf_event_stack_sample> ring_buffer_record;

  explicit SamplePerfEvent(uint64_t dyn_size)
      : ring_buffer_record{
            std::make_unique<dynamically_sized_perf_event_stack_sample>(
                dyn_size)} {}

  uint64_t GetTimestamp() const override {
    return ring_buffer_record->sample_id.time;
  }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record->sample_id.pid; }
  pid_t GetTid() const { return ring_buffer_record->sample_id.tid; }

  uint64_t GetStreamId() const {
    return ring_buffer_record->sample_id.stream_id;
  }

  uint32_t GetCpu() const { return ring_buffer_record->sample_id.cpu; }

  std::array<uint64_t, PERF_REG_X86_64_MAX> GetRegisters() const {
    return perf_event_sample_regs_user_all_to_register_array(
        ring_buffer_record->regs);
  }

  const char* GetStackData() const {
    return ring_buffer_record->stack.data.get();
  }
  char* GetStackData() { return ring_buffer_record->stack.data.get(); }
  uint64_t GetStackSize() const { return ring_buffer_record->stack.dyn_size; }

 private:
  static std::array<uint64_t, PERF_REG_X86_64_MAX>
  perf_event_sample_regs_user_all_to_register_array(
      const perf_event_sample_regs_user_all& regs) {
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
  perf_event_callchain_sample ring_buffer_record;
  std::vector<uint64_t> ips;
  explicit CallchainSamplePerfEvent(uint64_t callchain_size)
      : ips(callchain_size) {}

  uint64_t GetTimestamp() const override {
    return ring_buffer_record.sample_id.time;
  }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }
  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  uint64_t GetStreamId() const {
    return ring_buffer_record.sample_id.stream_id;
  }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }

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
  perf_event_sp_ip_8bytes_sample ring_buffer_record;

  uint64_t GetTimestamp() const override {
    return ring_buffer_record.sample_id.time;
  }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }
  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  uint64_t GetStreamId() const {
    return ring_buffer_record.sample_id.stream_id;
  }

  uint32_t GetCpu() const { return ring_buffer_record.sample_id.cpu; }

  // Get the stack pointer.
  uint64_t GetSp() const { return ring_buffer_record.regs.sp; }

  // Get the instruction pointer.
  uint64_t GetIp() const { return ring_buffer_record.regs.ip; }

  uint64_t GetReturnAddress() const {
    return ring_buffer_record.stack.top8bytes;
  }
};

class UretprobesPerfEvent : public PerfEvent, public AbstractUprobesPerfEvent {
 public:
  perf_event_ax_sample ring_buffer_record;

  uint64_t GetTimestamp() const override {
    return ring_buffer_record.sample_id.time;
  }

  void Accept(PerfEventVisitor* visitor) override;

  pid_t GetPid() const { return ring_buffer_record.sample_id.pid; }
  pid_t GetTid() const { return ring_buffer_record.sample_id.tid; }

  // Get AX register which holds integer return value.
  // See https://wiki.osdev.org/System_V_ABI.
  uint64_t GetAx() const { return ring_buffer_record.regs.ax; }

  uint64_t GetStreamId() const {
    return ring_buffer_record.sample_id.stream_id;
  }

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

class PerfEventSampleRaw {
 public:
  perf_event_sample_raw ring_buffer_record;
  std::vector<uint8_t> data;
  explicit PerfEventSampleRaw(uint32_t size) : data(size) {}
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_H_
