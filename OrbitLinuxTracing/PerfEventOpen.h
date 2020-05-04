#ifndef ORBIT_LINUX_TRACING_PERF_EVENT_OPEN_H_
#define ORBIT_LINUX_TRACING_PERF_EVENT_OPEN_H_

#include <OrbitBase/Logging.h>
#include <OrbitBase/SafeStrerror.h>
#include <asm/perf_regs.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <linux/version.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <ctime>

inline int perf_event_open(struct perf_event_attr* attr, pid_t pid, int cpu,
                           int group_fd, unsigned long flags) {
  return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

namespace LinuxTracing {

inline void perf_event_reset(int file_descriptor) {
  int ret = ioctl(file_descriptor, PERF_EVENT_IOC_RESET, 0);
  if (ret != 0) {
    ERROR("PERF_EVENT_IOC_RESET: %s", SafeStrerror(errno));
  }
}

inline void perf_event_enable(int file_descriptor) {
  int ret = ioctl(file_descriptor, PERF_EVENT_IOC_ENABLE, 0);
  if (ret != 0) {
    ERROR("PERF_EVENT_IOC_ENABLE: %s", SafeStrerror(errno));
  }
}

inline void perf_event_reset_and_enable(int file_descriptor) {
  perf_event_reset(file_descriptor);
  perf_event_enable(file_descriptor);
}

inline void perf_event_disable(int file_descriptor) {
  int ret = ioctl(file_descriptor, PERF_EVENT_IOC_DISABLE, 0);
  if (ret != 0) {
    ERROR("PERF_EVENT_IOC_DISABLE: %s", SafeStrerror(errno));
  }
}

inline void perf_event_redirect(int from_fd, int to_fd) {
  int ret = ioctl(from_fd, PERF_EVENT_IOC_SET_OUTPUT, to_fd);
  if (ret != 0) {
    ERROR("PERF_EVENT_IOC_SET_OUTPUT: %s", SafeStrerror(errno));
  }
}

inline uint64_t perf_event_get_id(int file_descriptor) {
  uint64_t id;
  int ret = ioctl(file_descriptor, PERF_EVENT_IOC_ID, &id);
  if (ret != 0) {
    ERROR("PERF_EVENT_IOC_ID: %s", SafeStrerror(errno));
    return 0;
  }
  return id;
}

// This must be in sync with struct perf_event_sample_id_tid_time_streamid_cpu
// in PerfEventRecords.h.
static constexpr uint64_t SAMPLE_TYPE_TID_TIME_STREAMID_CPU =
    PERF_SAMPLE_TID | PERF_SAMPLE_TIME | PERF_SAMPLE_STREAM_ID |
    PERF_SAMPLE_CPU;

// Sample all registers: they might all be necessary for DWARF-based stack
// unwinding.
// This must be in sync with struct perf_event_sample_regs_user_all in
// PerfEventRecords.h.
static constexpr uint64_t SAMPLE_REGS_USER_ALL =
    (1lu << PERF_REG_X86_AX) | (1lu << PERF_REG_X86_BX) |
    (1lu << PERF_REG_X86_CX) | (1lu << PERF_REG_X86_DX) |
    (1lu << PERF_REG_X86_SI) | (1lu << PERF_REG_X86_DI) |
    (1lu << PERF_REG_X86_BP) | (1lu << PERF_REG_X86_SP) |
    (1lu << PERF_REG_X86_IP) | (1lu << PERF_REG_X86_FLAGS) |
    (1lu << PERF_REG_X86_CS) | (1lu << PERF_REG_X86_SS) |
    (1lu << PERF_REG_X86_R8) | (1lu << PERF_REG_X86_R9) |
    (1lu << PERF_REG_X86_R10) | (1lu << PERF_REG_X86_R11) |
    (1lu << PERF_REG_X86_R12) | (1lu << PERF_REG_X86_R13) |
    (1lu << PERF_REG_X86_R14) | (1lu << PERF_REG_X86_R15);

// This must be in sync with struct perf_event_sample_regs_user_sp_ip in
// PerfEventRecords.h.
static constexpr uint64_t SAMPLE_REGS_USER_SP_IP =
    (1lu << PERF_REG_X86_SP) | (1lu << PERF_REG_X86_IP);

// This must be in sync with struct perf_event_ax_sample in
// PerfEventRecords.h.
static constexpr uint64_t SAMPLE_REGS_USER_AX = (1lu << PERF_REG_X86_AX);

// Max to pass to perf_event_open without getting an error is (1u << 16u) - 8,
// because the kernel stores this in a short and because of alignment reasons.
// But the size the kernel actually returns is smaller, because the maximum size
// of the entire record the kernel is willing to return is (1u << 16u) - 8.
// If we want the size we pass to coincide with the size we get, we need to pass
// a lower value. For the current layout of perf_event_stack_sample, the maximum
// size is 65312, but let's leave some extra room.
// TODO: As this amount of memory has to be copied from the ring buffer for each
//  sample, this constant should be a parameters and should be made available in
//  some setting.
static constexpr uint16_t SAMPLE_STACK_USER_SIZE = 65000;

static_assert(sizeof(void*) == 8);
static constexpr uint16_t SAMPLE_STACK_USER_SIZE_8BYTES = 8;

// perf_event_open for context switches.
int context_switch_event_open(pid_t pid, int32_t cpu);

// perf_event_open for task (fork and exit) and mmap records in the same buffer.
int mmap_task_event_open(pid_t pid, int32_t cpu);

// perf_event_open for stack sampling.
int sample_event_open(uint64_t period_ns, pid_t pid, int32_t cpu);

// perf_event_open for stack sampling using frame pointers.
int callchain_sample_event_open(uint64_t period_ns, pid_t pid, int32_t cpu);

// perf_event_open for uprobes and uretprobes.
int uprobes_retaddr_event_open(const char* module, uint64_t function_offset,
                               pid_t pid, int32_t cpu);

int uprobes_stack_event_open(const char* module, uint64_t function_offset,
                             pid_t pid, int32_t cpu);

int uretprobes_event_open(const char* module, uint64_t function_offset,
                          pid_t pid, int32_t cpu);

// Create the ring buffer to use perf_event_open in sampled mode.
void* perf_event_open_mmap_ring_buffer(int fd, uint64_t mmap_length);

// perf_event_open for tracepoint events. This opens a perf event for the
// tracepoint given by the category (for example, "sched") and the name
// (for example, "sched_waking"). Returns the file descriptor for the
// perf event or -1 in case of any errors.
int tracepoint_event_open(const char* tracepoint_category,
                          const char* tracepoint_name, pid_t pid, int32_t cpu);

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_PERF_EVENT_OPEN_H_
