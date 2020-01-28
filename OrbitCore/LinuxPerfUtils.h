//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include <asm/perf_regs.h>
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <linux/version.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>

//-----------------------------------------------------------------------------
namespace LinuxPerfUtils {
// This must be in sync with the struct perf_sample_id defined below.
static constexpr uint64_t SAMPLE_TYPE_FLAGS =
    PERF_SAMPLE_TID | PERF_SAMPLE_TIME | PERF_SAMPLE_CPU;

// TODO: These constants should be made available in some settings.
// Max to pass to perf_event_open without getting an error is (1u << 16u) - 8,
// because the kernel stores this in a short and because of alignment reasons.
// But the size the kernel actually returns is smaller, because the maximum
// size of the entire record the kernel is willing to return is (1u << 16u) - 8.
// If we want the size we pass to coincide with the size we get, we need to
// pass a lower value. For the current layout of perf_record_with_stack,
// the maximum size is 65312, but let's leave some extra room.
static constexpr uint16_t SAMPLE_STACK_USER_SIZE = 65000;

// Sample stack pointer and instruction pointer only.
static constexpr uint64_t SAMPLE_REGS_USER_SP_IP =
    (1lu << PERF_REG_X86_SP) | (1lu << PERF_REG_X86_IP);

// Sample all pointers: they might all be necessary later for DWARF-based stack
// unwinding.
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

// This struct must be in sync with the SAMPLE_TYPE_FLAGS, as the bits set
// in perf_event_attr::sample_type determine the fields this struct should have.
struct perf_sample_id {
  uint32_t pid, tid; /* if PERF_SAMPLE_TID */
  uint64_t time;     /* if PERF_SAMPLE_TIME */
  uint32_t cpu, res; /* if PERF_SAMPLE_CPU */
};

struct perf_sample_regs_user_sp_ip {
  uint64_t abi;
  uint64_t sp;
  uint64_t ip;
};

struct perf_sample_regs_user_all {
  uint64_t abi;
  uint64_t ax;
  uint64_t bx;
  uint64_t cx;
  uint64_t dx;
  uint64_t si;
  uint64_t di;
  uint64_t bp;
  uint64_t sp;
  uint64_t ip;
  uint64_t flags;
  uint64_t cs;
  uint64_t ss;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
};
struct perf_sample_stack_user {
  uint64_t size;                     /* if PERF_SAMPLE_STACK_USER */
  char data[SAMPLE_STACK_USER_SIZE]; /* if PERF_SAMPLE_STACK_USER */
  uint64_t dyn_size; /* if PERF_SAMPLE_STACK_USER && size != 0 */
};

inline int32_t perf_event_open(struct perf_event_attr* a_HWEvent, pid_t a_PID,
                               int32_t a_CPU, int32_t a_GroupFD,
                               uint32_t a_Flags) {
  return syscall(__NR_perf_event_open, a_HWEvent, a_PID, a_CPU, a_GroupFD,
                 a_Flags);
}

inline void start_capturing(uint32_t a_FileDescriptor) {
  ioctl(a_FileDescriptor, PERF_EVENT_IOC_RESET, 0);
  ioctl(a_FileDescriptor, PERF_EVENT_IOC_ENABLE, 0);
}

inline void stop_capturing(uint32_t a_FileDescriptor) {
  ioctl(a_FileDescriptor, PERF_EVENT_IOC_DISABLE, 0);
}

perf_event_attr generic_perf_event_attr();

// perf_event_open for fork and exit.
int32_t task_event_open(int32_t a_CPU);

// perf_event_open for context switches.
int32_t context_switch_open(pid_t a_PID, int32_t a_CPU);

// perf_event_open for kernel tracepoints.
int32_t tracepoint_event_open(uint64_t a_TracepointID, pid_t a_PID,
                              int32_t a_CPU,
                              uint64_t additional_sample_type = 0);

// perf_event_open for stack sampling
int32_t stack_sample_event_open(pid_t a_PID, uint64_t a_Frequency);

// perf_event_open for uprobes.
bool supports_perf_event_uprobes();
int32_t uprobe_event_open(const char* a_Module, uint64_t a_FunctionOffset,
                          pid_t a_PID, int32_t a_CPU, int32_t a_GroupFd = -1,
                          uint64_t additonal_sample_type = 0);
int32_t uretprobe_event_open(const char* a_Module, uint64_t a_FunctionOffset,
                             pid_t a_PID, int32_t a_CPU, int32_t a_GroupFd = -1,
                             uint64_t additonal_sample_type = 0);
}  // namespace LinuxPerfUtils
