#include "PerfEventUtils.h"

#include <linux/perf_event.h>

#include <cerrno>
#include <cstring>

#include "Logging.h"

namespace LinuxTracing {
namespace {
perf_event_attr generic_event_attr() {
  perf_event_attr pe{};
  pe.size = sizeof(struct perf_event_attr);
  pe.sample_period = 1;
  pe.use_clockid = 1;
  pe.clockid = CLOCK_MONOTONIC;
  pe.sample_id_all = 1;  // also include timestamps for lost events
  pe.disabled = 1;

  // We can set these even if we do not do sampling, as without the
  // PERF_SAMPLE_STACK_USER or PERF_SAMPLE_REGS_USER flags being set in
  // perf_event_attr::sample_type they will not be used anyways.
  pe.sample_stack_user = SAMPLE_STACK_USER_SIZE;
  pe.sample_regs_user = SAMPLE_REGS_USER_ALL;

  pe.sample_type = SAMPLE_TYPE_BASIC_FLAGS;

  return pe;
}

int32_t generic_event_open(perf_event_attr* attr, pid_t pid, int32_t cpu) {
  int32_t fd = perf_event_open(attr, pid, cpu, -1, 0);
  if (fd == -1) {
    ERROR("perf_event_open: %s", strerror(errno));
  }
  return fd;
}

perf_event_attr uprobe_event_attr(const char* module,
                                  uint64_t function_offset) {
  perf_event_attr pe = generic_event_attr();

  pe.type = 7;  // TODO: should be read from
                //  "/sys/bus/event_source/devices/uprobe/type"
  pe.config1 =
      reinterpret_cast<uint64_t>(module);  // pe.config1 == pe.uprobe_path
  pe.config2 = function_offset;            // pe.config2 == pe.probe_offset

  return pe;
}
}  // namespace

int32_t task_event_open(int32_t cpu) {
  perf_event_attr pe = generic_event_attr();
  pe.type = PERF_TYPE_SOFTWARE;
  pe.config = PERF_COUNT_SW_DUMMY;
  pe.task = 1;

  return generic_event_open(&pe, -1, cpu);
}

int32_t pid_context_switch_event_open(pid_t pid) {
  perf_event_attr pe = generic_event_attr();
  pe.type = PERF_TYPE_SOFTWARE;
  pe.config = PERF_COUNT_SW_DUMMY;
  pe.context_switch = 1;

  return generic_event_open(&pe, pid, -1);
}

int32_t cpu_context_switch_event_open(int32_t cpu) {
  perf_event_attr pe = generic_event_attr();
  pe.type = PERF_TYPE_SOFTWARE;
  pe.config = PERF_COUNT_SW_DUMMY;
  pe.context_switch = 1;

  return generic_event_open(&pe, -1, cpu);
}

int32_t sample_mmap_task_event_open(pid_t pid, uint64_t period_ns) {
  perf_event_attr pe = generic_event_attr();
  pe.type = PERF_TYPE_SOFTWARE;
  pe.config = PERF_COUNT_SW_CPU_CLOCK;
  pe.sample_period = period_ns;
  pe.sample_type |= PERF_SAMPLE_STACK_USER | PERF_SAMPLE_REGS_USER;
  // Also record mmaps, ...
  pe.mmap = 1;
  // ... forks, and termination.
  pe.task = 1;

  return generic_event_open(&pe, pid, -1);
}

int32_t uprobe_event_open(const char* module, uint64_t function_offset,
                          pid_t pid, int32_t cpu) {
  perf_event_attr pe = uprobe_event_attr(module, function_offset);
  pe.config = 0;

  return generic_event_open(&pe, pid, cpu);
}

int32_t uprobe_stack_event_open(const char* module, uint64_t function_offset,
                                pid_t pid, int32_t cpu) {
  perf_event_attr pe = uprobe_event_attr(module, function_offset);
  pe.config = 0;
  pe.sample_type |= PERF_SAMPLE_STACK_USER | PERF_SAMPLE_REGS_USER;

  return generic_event_open(&pe, pid, cpu);
}

int32_t uretprobe_event_open(const char* module, uint64_t function_offset,
                             pid_t pid, int32_t cpu) {
  perf_event_attr pe = uprobe_event_attr(module, function_offset);
  pe.config = 1;  // Set bit 0 of config for uretprobe.

  return generic_event_open(&pe, pid, cpu);
}

int32_t uretprobe_stack_event_open(const char* module, uint64_t function_offset,
                                   pid_t pid, int32_t cpu) {
  perf_event_attr pe = uprobe_event_attr(module, function_offset);
  pe.config = 1;  // Set bit 0 of config for uretprobe.
  pe.sample_type |= PERF_SAMPLE_STACK_USER | PERF_SAMPLE_REGS_USER;

  return generic_event_open(&pe, pid, cpu);
}

}  // namespace LinuxTracing
